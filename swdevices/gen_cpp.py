#!/usr/bin/env python

# Copyright 2019 HP Development Company, L.P.
# SPDX-License-Identifier: MIT

import sys
import os
import json
from collections import OrderedDict
from functools import reduce
import jsonschema as js


header_str = """
// {copyright}

#ifndef SWDEVICES_INCLUDE_{KLASS}_H_
#define SWDEVICES_INCLUDE_{KLASS}_H_

#include <hippo_swdevice.h>

namespace hippo {{
{structs}

class {klass} : public HippoSwDevice {{
 public:
  using HippoSwDevice::disconnect_device;

  {klass}();
  explicit {klass}(uint32_t device_index);
  {klass}(const char *address, uint32_t port);
  {klass}(const char *address, uint32_t port, uint32_t device_index);
  virtual ~{klass}(void);

  uint64_t connect_device(void);

  // to use the sw device as client

{methods}
  // override to implement sw device server callbacks

{callbacks}
 protected:
  uint64_t ProcessCommand(const char *method, void *params,
                          void *result) override;
{callbacks_priv}
  // {klass}'s types parsers/generators
{parsers}}};

}}   // namespace hippo

#endif   // SWDEVICES_INCLUDE_{KLASS}_H_
"""

struct_str = """
typedef struct {name} {{
{members}}} {name};
"""

member_str = """  {} {};
"""

c2json_str = """  uint64_t {type}_c2json(const {type} &set,
                   {padding}void *obj);
"""

json2c_str = """  uint64_t {type}_json2c(const void *obj,
                   {padding}{type} *get);
"""

source_str = """
// {copyright}

#include <json.hpp>
#include \"../include/{name}.h\"

namespace nl = nlohmann;

namespace hippo {{

const char devName[] = "{name}";

{klass}::{klass}() :
    HippoSwDevice(devName) {{
  ADD_FILE_TO_MAP();
}}

{klass}::{klass}(uint32_t device_index) :
    HippoSwDevice(devName, device_index) {{
}}

{klass}::{klass}(const char *address, uint32_t port) :
    HippoSwDevice(devName, address, port) {{
}}

{klass}::{klass}(const char *address, uint32_t port,
                         uint32_t device_index) :
    HippoSwDevice(devName, address, port, device_index) {{
}}

{klass}::~{klass}(void) {{
}}

// member functions implementing the client side of the sw device
{methods}
// member functions implementing the server side of the sw device
const char {klass}_device_json[] =
{json}

uint64_t {klass}::connect_device(void) {{
  uint64_t err = 0LL;
  if (err = HippoSwDevice::connect_device({klass}_device_json)) {{
    return err;
  }}
  return 0LL;
}}
{process_cmd}
// callbacks
{callbacks}// callbacks private
{callbacks_priv}// {klass}'s types parser/generators
{parsers}
}}    // namespace hippo
"""

src_c2json_str = (
    """uint64_t {klass}::{type}_c2json(const {type} &set, void *obj) {{
  if (obj == NULL) {{
    return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_INVALID_PARAM);
  }}
  nl::json params;{members}
  *(reinterpret_cast<nl::json*>(obj)) = params;
  return HIPPO_OK;
}}

""")

src_c2json_aux_str = """
  nl::json {name};
  if ({type}_c2json(set.{name}, &{name})) {{
    return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_INVALID_PARAM);
  }}
  params[\"{name}\"] = {name};
"""

src_json2c_str = (
    """uint64_t {klass}::{type}_json2c(const void *obj, {type} *get) {{
  uint64_t err = HIPPO_OK;
  if (obj == NULL || get == NULL) {{
    return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_INVALID_PARAM);
  }}
  const nl::json *j = reinterpret_cast<const nl::json*>(obj);
  if (!j->is_object()) {{
    return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_MESSAGE_ERROR);
  }}
  const void *j_obj = NULL;
  try {{
{members}  }} catch (nl::json::exception) {{     // out_of_range or type_error
    return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_ERROR);
  }}
  return err;
}}

""")

src_json2c_aux_str = \
"""    j_obj = reinterpret_cast<const void*>(&j->at("{name}"));
    if ({type}_json2c(j_obj, &(get->{name}))) {{
      return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_INVALID_PARAM);
    }}
"""

method_src = """
{sig}) {{
  uint64_t err = 0LL;

  // parse parameters
{c2json}{param}
  // send command
  nl::json jget;
  void *jget_ptr = reinterpret_cast<void*>(&jget);

  // std::string s = reinterpret_cast<nl::json*>(jset_ptr)->dump();
  // fprintf(stderr, "SendRawMsg('%s')\\n", s.c_str());
  int timeout = {timeout};
  if (err = SendRawMsg("{method}", jset_ptr, timeout, jget_ptr)) {{
    return err;
  }}
  // get the results
  // s = reinterpret_cast<nl::json*>(jget_ptr)->dump();
  // fprintf(stderr, "  Response('%s')\\n", s.c_str());
{json2c}
  return err;
}}
"""

method_c2json_src = """  nl::json j{name};
  void *j{name}_ptr = reinterpret_cast<void*>(&j{name});
  if (err = {type}_c2json({name}, j{name}_ptr)) {{
    return err;
  }}
"""

method_json2c_src = """  if ({name} != NULL) {{
    err = {type}_json2c(jget_ptr, {name});
  }}"""

multi_param_str = """  // create a list with all parameters
  nl::json jset;
{param}  void *jset_ptr = reinterpret_cast<void*>(&jset);
"""

multi_param_item = """  jset.push_back(j{name});
"""

process_cmd_str = """
uint64_t {klass}::ProcessCommand(const char *method, void *param, void *result) {{
  typedef uint64_t ({klass}::*cb)(void*, void*);

  std::map<std::string, cb> map = {{
{methods}  }};
  return (this->*(map[method]))(param, result);
}}
"""

callback_src_str = """{sig}) {{
  return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_FUNC_NOT_AVAILABLE);
}}
"""

callback_priv_str = """uint64_t {klass}::{method}_cb_p(void *param, void *result) {{
  uint64_t err = 0LL;
  nl::json *params = reinterpret_cast<nl::json*>(param);

{callback_j2c}{callback_call_def}
  if (err = {callback_call}) {{
    return err;
  }}{callback_c2j}  return err;
}}

"""

callback_j2c_str = """  {type} {name};
  try {{
    if (err = {type}_json2c(&params->at({idx}), &{name})) {{
      return err;
    }}
  }} catch (nl::json::exception) {{     // out_of_range or type_error
    return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_INVALID_PARAM);
  }}
"""

callback_c2j_str = """
  if (err = {type}_c2json({name}, result)) {{
    return err;
  }}
"""


#
# generating functions
#
# string, integer, number, object, array, boolean, null

schema2c = {
    'string' : 'wcharptr',
    'integer' : 'int32_t',
    'number' : 'float',
    #'array': ''
    'boolean' : 'bool',
    'b64bytes': 'b64bytes'
}


def parse_schema(st, depth, schema_file_name, all_types):
    schema_file_name = os.path.abspath(schema_file_name)
    # do not add the types that already exist due to other methods
    if schema_file_name in all_types:
        return
    else:
        all_types.append(schema_file_name)

    # need to verify the schemas are valid
    # print('>>> ', schema_file_name)

    with open(schema_file_name) as fp:
        schema = json.load(fp)

    #
    # schema_path = 'file:///' + os.path.abspath('.').replace("\\", "/") + '/'
    # resolver = js.RefResolver(schema_path, None)
    # js.validate(data, schema, resolver=resolver)
    #
    name = schema['$id']
    name = name[:name.index('.schema.json')]
    st[name] = {'depth' : depth, 'members' : []}
    for k, v in schema['properties'].items():
        if 'type' in v:
            st[name]['members'].append({'name': k,
                                        'type': schema2c[v['type']]})
        elif '$ref' in v:
            if v['$ref'] not in st:
                parse_schema(st, depth+1,
                             os.path.join(os.path.dirname(schema_file_name),
                                          v['$ref']),
                             all_types)

            n = v['$ref']
            n = n[:n.index('.schema.json')]
            st[name]['members'].append({'name': k, 'type': n})

#
# generate header code
#
def generate_header(klass, st):
    header = ''
    funcs = ''

    for k in sorted(st, key=lambda x: st[x]['depth'], reverse=True):
        v = st[k]
        members = ""
        for m in v['members']:
            members = members + member_str.format(m['type'],
                                                  m['name'])
        x = struct_str.format(name=k, members=members)
        header = header + x
        padding = len(k) * ' '
        funcs = funcs + c2json_str.format(type=k, padding=padding)
        funcs = funcs + json2c_str.format(type=k, padding=padding)

    src = ''
    for k in sorted(st, key=lambda x: st[x]['depth'], reverse=True):
        v = st[k]
        members_c2j = ""
        members_j2c = ""
        for m in v['members']:
            members_c2j = (members_c2j +
                           src_c2json_aux_str.format(type=m['type'],
                                                     name=m['name']))
            members_j2c = (members_j2c +
                           src_json2c_aux_str.format(type=m['type'],
                                                     name=m['name']))
        src = src + src_c2json_str.format(type=k,
                                          members=members_c2j,
                                          klass=klass)
        src = src + src_json2c_str.format(type=k,
                                          members=members_j2c,
                                          klass=klass)
    return header, funcs, src


#
def get_type_conversions(klass, ptype, path, all_types):

    schema_file_name = os.path.join(path,    # '..',
                                    'schemas',
                                    ptype + '.schema.json')

    # print(os.path.dirname(schema_file_name))
    st = {}
    depth = 0
    parse_schema(st, depth, schema_file_name, all_types)
    return generate_header(klass, st)


###########

if __name__ == '__main__':
    # JSON file describing the SW device
    swdev_file_name = os.path.abspath(sys.argv[1])
    # path to all files describing schemas
    schemas_path = os.path.abspath(sys.argv[2])

    with open(swdev_file_name) as fp:
        swdev = json.load(fp)
    #
    # validate the parameter file against a DeviceConnected schema
    schema_file_name = os.path.join(os.path.abspath(schemas_path),
                                    "DeviceConnected.schema.json")
    with open(schema_file_name) as fp:
        schema = json.load(fp)
    schema_path = ('file:///' +
                   os.path.split(schema_file_name)[0].replace("\\", "/") +
                   '/')
    resolver = js.RefResolver(schema_path, None)
    js.validate(swdev, schema, resolver=resolver)
    #
    swdev_name = swdev['device_name']
    klass = ''.join(c.upper() if i == 0 else c for i, c in enumerate(swdev_name))
    swdev_api = swdev['api']
    # print(swdev_name, swdev_api)
    # print(swdev)
    # modify sw device json definition to go into c++ file as string
    swdev['device_name'] = swdev['device_name'].lower()
    # so we have always the same order
    swdev_o = OrderedDict([('device_name', swdev['device_name']),
                           ('api', swdev['api'])])
    sw_json = json.dumps(swdev_o, indent=1)
    sw_json = sw_json.replace('"', '\\"')
    sw_json_lst = sw_json.split('\n')
    sw_json_lst[0] = '[' + sw_json_lst[0]
    sw_json_lst[-1] += ']'
    for i in range(len(sw_json_lst)):
        sw_json_lst[i] = '"'+sw_json_lst[i]+'"\n'
    sw_json_lst[-1] = sw_json_lst[-1][:-2] + '\";'
    sw_json = ''.join(sw_json_lst)
    # print(sw_json)
    #
    # generate header file
    #
    param_types = set()

    # public methods
    methods = []
    for m in swdev_api:
        doc_list = m['doc'] if 'doc' in m else ['<no documentation found>']
        doc = ''.join(map(lambda x: '  // {}\n'.format(x), doc_list))
        pm = {'class' : klass,
              'ret' : 'uint64_t',
              'method' : m['method'],
              'timeout': m['timeout'],
              'doc' : doc,
              'in' : [],
              'out' : []}

        for i, p in enumerate(m['params']):
            p_type = p['type']
            try:
                p_type = schema2c[p_type]
            except:
                param_types.add(p_type)
            pm['in'].append((p_type, p['name']))

        for r in m['result']:
            r_type = r['type']
            try:
                r_type = schema2c[r_type]
            except:
                param_types.add(r_type)
            pm['out'].append((r_type, r['name']))
        methods.append(pm)

    # public methods header string
    methods_hdr_str = ''
    for m in methods:
        s = '  {ret} {method}('.format(**m)
        len_s = len(s)
        for i, p in enumerate(m['in']):
            padding = len_s * ' ' if i is not 0 else ''
            s = s + '{0}const {1} &{2},\n'.format(padding,
                                                  p[0],
                                                  p[1])
        padding = len_s * ' ' if m['in'] else ''
        for i, p in enumerate(m['out']):
            s = s + '{0}{1} *{2},\n'.format(padding,
                                            p[0],
                                            p[1])
        methods_hdr_str = (methods_hdr_str + m['doc'] +
                           (s[:-2] if s[-2] == ',' else s) + ');\n')

    # public callbacks
    callbacks_hdr_str = ''
    for m in methods:
        s = '  virtual {ret} {method}_cb('.format(**m)
        len_s = len(s)
        for i, p in enumerate(m['in']):
            padding = len_s * ' ' if i is not 0 else ''
            s = s + '{0}const {1} &{2},\n'.format(padding,
                                                  p[0],
                                                  p[1])
        padding = len_s * ' ' if m['in'] else ''
        for i, p in enumerate(m['out']):
            s = s + '{0}{1} *{2},\n'.format(padding,
                                            p[0],
                                            p[1])
        callbacks_hdr_str = (callbacks_hdr_str + m['doc'] +
                             (s[:-2] if s[-2] == ',' else s) + ');\n')


    # protected callbacks
    callbacks_priv_hdr_str = ''
    for m in methods:
        callbacks_priv_hdr_str = (
            callbacks_priv_hdr_str +
            '  {ret} {method}_cb_p(void *param, void *result);\n'.format(**m))


    # public methods src string
    methods_src_str = ''
    for m in methods:
        s = '{ret} {class}::{method}('.format(**m)
        len_s = len(s)
        src_c2json = ''
        src_json2c = ''
        param = ''
        for i, p in enumerate(m['in']):
            padding = len_s*' ' if i is not 0 else ''
            s = s + '{0}const {1} &{2},\n'.format(padding,
                                                  p[0],
                                                  p[1])
            src_c2json = src_c2json + method_c2json_src.format(type=p[0],
                                                               name=p[1])
            param = param + multi_param_item.format(name=p[1])
        #
        for i, p in enumerate(m['out']):
            padding = len_s * ' ' if m['in'] else ''
            s = s + '{0}{1} *{2},\n'.format(padding,
                                            p[0],
                                            p[1])
            src_json2c = src_json2c + method_json2c_src.format(type=p[0],
                                                               name=p[1])

        param = multi_param_str.format(param=param)
        methods_src_str = (methods_src_str +
                           method_src.format(sig=s[:-2] if s[-2] == ',' else s,
                                             c2json=src_c2json,
                                             json2c=src_json2c,
                                             method=m['method'],
                                             timeout=m['timeout'],
                                             param=param))

    # process cmd
    methods_str = ''
    # process_switch = ''
    for i, m in enumerate(swdev_api):
        methods_str = (methods_str +
                       "    {{\"{m}\", &{klass}::{m}_cb_p}},\n".format(
                           m=m['method'],
                           klass=klass))
    process_cmd = process_cmd_str.format(klass=klass,
                                         methods=methods_str,
                                         num_methods=len(swdev_api)-1)

    # callback
    callback_str = ''
    for m in methods:
        s = '{ret} {class}::{method}_cb('.format(**m)
        len_s = len(s)
        src_c2json = ''
        src_json2c = ''
        param = ''
        for i, p in enumerate(m['in']):
            padding = len_s*' ' if i is not 0 else ''
            s = s + '{0}const {1} &,\n'.format(padding,
                                               p[0])
            src_c2json = src_c2json + method_c2json_src.format(type=p[0],
                                                               name=p[1])
            param = param + multi_param_item.format(name=p[1])
        #
        padding = len_s * ' ' if len(m['in']) is not 0 else ''
        for i, p in enumerate(m['out']):
            s = s + '{0}{1} *,\n'.format(padding,
                                         p[0])
            src_json2c = src_json2c + method_json2c_src.format(type=p[0],
                                                               name=p[1])
        callback_str = callback_str + callback_src_str.format(
            sig=s[:-2] if s[-2] == ',' else s) + '\n'

    # callback priv
    callback_priv = ''
    for m in methods:
        callback_j2c = ''
        callback_call_def = ''
        callback_call = '{}_cb('.format(m['method'])
        callback_c2j = ''
        free = ''
        for i, p in enumerate(m['in']):
            callback_j2c += callback_j2c_str.format(type=p[0],
                                                    name=p[1],
                                                    idx=i)
            callback_call += '{}, '.format(p[1])

        for i, p in enumerate(m['out']):
            callback_c2j += callback_c2j_str.format(type=p[0], name=p[1])
            callback_call += '&{}, '.format(p[1])
            callback_call_def += '  {type} {name};'.format(
                type=p[0], name=p[1])

        callback_call = (callback_call[:-2]
                         if callback_call[-2] == ','
                         else callback_call) + ')'

        callback_priv += (
            callback_priv_str.format(klass=klass,
                                     method=m['method'],
                                     callback_j2c=callback_j2c,
                                     callback_call_def=callback_call_def,
                                     callback_call=callback_call,
                                     callback_c2j=callback_c2j))
    # main
    path = os.path.dirname(os.path.realpath(__file__))

    header = ''
    funcs = ''
    src = ''
    all_types = []
    for p in sorted(param_types):
        h, f, s = get_type_conversions(klass, p, path, all_types)
        all_types.append(p)
        header = header + h
        funcs = funcs + f
        src = src + s

    cright = reduce(lambda x, y: '{}\n// {}\n'.format(x, y),
                    swdev['copyright'])
    all_header = header_str.format(copyright=cright,
                                   KLASS=klass.upper(),
                                   klass=klass,
                                   methods=methods_hdr_str,
                                   structs=header,
                                   callbacks=callbacks_hdr_str,
                                   callbacks_priv=callbacks_priv_hdr_str,
                                   parsers=funcs)

    # print(all_header)
    header_file = os.path.join(path,    # '..',
                               'include',
                               '{}.h'.format(klass.lower()))
    os.makedirs(os.path.dirname(header_file), exist_ok=True)
    with open(header_file, 'w') as fp:
        fp.write(all_header)

    #
    all_src = source_str.format(copyright=cright,
                                klass=klass,
                                name=klass.lower(),
                                methods=methods_src_str,
                                process_cmd=process_cmd,
                                callbacks=callback_str,
                                callbacks_priv=callback_priv,
                                json=sw_json,
                                parsers=src)
    # print(all_src)
    source_file = os.path.join(path,    # '..',
                               'src',
                               '{}.cc'.format(klass.lower()))
    with open(source_file, 'w') as fp:
        fp.write(all_src)
