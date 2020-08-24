#!/usr/bin/env python

# Copyright 2019 HP Development Company, L.P.
# SPDX-License-Identifier: MIT

"""This script reads and parses a file containing a SW device API definition
using the json schema (http://json-schema.org/) specification and generates
the header (.h) and source (.cc) files that define a c++ class implementing
the schema API including all the type conversion functions from c++ to
SoHal's jsonrpc 2.0 that are called automatically"""

import os
import json
import argparse
from collections import OrderedDict
from functools import reduce
import jsonschema as js


#
# how to translate schema types to c++
#
SCHEMA_2_C = {
    'string' : 'wcharptr',
    'integer' : 'int32_t',
    'number' : 'float',
    'boolean' : 'bool',
    'b64bytes': 'b64bytes'
}

def indent_func_def(func_def):
    """Ensures max columns in a function signature follows style guide"""
    if len(func_def) < 80:
        return func_def
    parts = func_def.split(',')
    idx = func_def.index('(')
    params = parts[0]
    for x in parts[1:]:
        params += ',\n{}{}'.format(idx * ' ', x)
    return params

#
def get_swdev(file_name, schemas_dir):
    """Reads and validates the sw definition schema file"""
    # JSON file describing the SW device
    swdev_file_name = os.path.abspath(file_name)
    # path to all files describing schemas
    schemas_path = os.path.abspath(schemas_dir)
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
    swdev['klass'] = (''.join(c.upper() if i == 0 else c for i, c in
                              enumerate(swdev['device_name'])))
    swdev['device_name'] = swdev['device_name'].lower()
    return swdev

#
# modify sw device json definition to go into c++ file as string
# so we have always the same order
def get_sw_json(swdev):
    """Converts the data in the schema file into a string that can be sent
    to SoHal's device_connected API
    """
    swdev_o = OrderedDict([('device_name', swdev['device_name']),
                           ('api', swdev['api'])])
    sw_json = json.dumps(swdev_o, indent=1)
    sw_json = sw_json.replace('"', '\\"')
    sw_json_lst = sw_json.split('\n')
    sw_json_lst[0] = '[' + sw_json_lst[0]
    sw_json_lst[-1] += ']'
    sw_json_lst = list(map('"{}"\n'.format, sw_json_lst))
    sw_json_lst[-1] = sw_json_lst[-1][:-2] + '\";'
    sw_json = ''.join(sw_json_lst)
    return sw_json

# public methods
def get_methods(klass, swdev_api):
    """Returns a dictionary with the data read from the sw device schema file
    and a dictionary with the types of the parameters
    """
    param_types = set()
    methods = []
    for m in swdev_api:
        if m['timeout'] < 0:
            raise  Exception('Timeout for method \"'+m['method']+ \
                             '\" is '+ str(m['timeout']) +' which is invalid.')
        doc_list = m['doc'] if 'doc' in m else ['<no documentation found>']
        doc = ''.join(map('  // {}\n'.format, doc_list))
        pm = {'class' : klass,
              'ret' : 'uint64_t',
              'method' : m['method'],
              'timeout': m['timeout'],
              'doc' : doc,
              'in' : [],
              'out' : []}
        for p in m['params']:
            p_type = p['type']
            try:
                p_type = SCHEMA_2_C[p_type]
            except KeyError:
                param_types.add(p_type)
            pm['in'].append((p_type, p['name']))
        for r in m['result']:
            r_type = r['type']
            try:
                r_type = SCHEMA_2_C[r_type]
            except KeyError:
                param_types.add(r_type)
            pm['out'].append((r_type, r['name']))
        methods.append(pm)
    return methods, param_types


#
# string template for full sw device class definition header (.h)
#
HEADER_STR = """
// {copyright}

#ifndef SWDEVICES_INCLUDE_{KLASS}_H_
#define SWDEVICES_INCLUDE_{KLASS}_H_

#include <include/hippo_swdevice.h>

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

#
# string template for full sw device class function implementation (.cc)
#
SOURCE_STR = """
// {copyright}
#include <include/json.hpp>
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
  ADD_FILE_TO_MAP();
}}

{klass}::{klass}(const char *address, uint32_t port) :
    HippoSwDevice(devName, address, port) {{
  ADD_FILE_TO_MAP();
}}

{klass}::{klass}(const char *address, uint32_t port,
                         uint32_t device_index) :
    HippoSwDevice(devName, address, port, device_index) {{
  ADD_FILE_TO_MAP();
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

#
# generating functions
#
def parse_schema(structs, depth, schema_file_name, all_types):
    """Parses the schema file and fills up a dictionary with its content
    """
    schema_file_name = os.path.abspath(schema_file_name)
    # do not add the types that already exist due to other methods
    if schema_file_name in all_types:
        return

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
    structs[name] = {'depth' : depth, 'members' : []}
    for k, v in schema['properties'].items():
        if 'type' in v:
            structs[name]['members'].append({'name': k,
                                             'type': SCHEMA_2_C[v['type']]})
        elif '$ref' in v:
            if v['$ref'] not in structs:
                parse_schema(structs, depth+1,
                             os.path.join(os.path.dirname(schema_file_name),
                                          v['$ref']),
                             all_types)

            n = v['$ref']
            n = n[:n.index('.schema.json')]
            structs[name]['members'].append({'name': k, 'type': n})

#
# generate structs, header and type conversions
#
SRC_JSON2C_AUX_STR = \
"""    j_obj = reinterpret_cast<const void*>(&j->at("{name}"));
    if ({type}_json2c(j_obj, &(get->{name}))) {{
      return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_INVALID_PARAM);
    }}
"""

SRC_JSON2C_DEF_STR = (
    """uint64_t {klass}::{type}_json2c(const void *obj, {type} *get) {{""")

SRC_JSON2C_STR = (
    """{func_def}
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

SRC_C2JSON_AUX_STR = """
  nl::json {name};
  if ({type}_c2json(set.{name}, &{name})) {{
    return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_INVALID_PARAM);
  }}
  params[\"{name}\"] = {name};
"""

SRC_C2JSON_DEF_STR = (
    """uint64_t {klass}::{type}_c2json(const {type} &set, void *obj) {{""")

SRC_C2JSON_STR = (
    """{func_def}
  if (obj == NULL) {{
    return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_INVALID_PARAM);
  }}
  nl::json params;{members}
  *(reinterpret_cast<nl::json*>(obj)) = params;
  return HIPPO_OK;
}}

""")

C2JSON_STR = """  uint64_t {type}_c2json(const {type} &set,
                   {padding}void *obj);
"""

JSON2C_STR = """  uint64_t {type}_json2c(const void *obj,
                   {padding}{type} *get);
"""

STRUCT_STR = """
typedef struct {name} {{
{members}}} {name};
"""


def generate_header_funcs_src(klass, structs):
    """ Goes through the structs dictionary and returns the structs
    definitions (in structs), the type conversion function headers (in funcs)
    and source code (in src)
    """
    header = ''
    funcs = ''
    for k in sorted(structs, key=lambda x: structs[x]['depth'], reverse=True):
        v = structs[k]
        members = ""
        for m in v['members']:
            members = members + '  {} {};\n'.format(m['type'], m['name'])
        x = STRUCT_STR.format(name=k, members=members)
        header = header + x
        padding = len(k) * ' '
        funcs = funcs + C2JSON_STR.format(type=k, padding=padding)
        funcs = funcs + JSON2C_STR.format(type=k, padding=padding)

    src = ''
    for k in sorted(structs, key=lambda x: structs[x]['depth'], reverse=True):
        v = structs[k]
        members_c2j = ""
        members_j2c = ""
        for m in v['members']:
            members_c2j = (members_c2j +
                           SRC_C2JSON_AUX_STR.format(type=m['type'],
                                                     name=m['name']))
            members_j2c = (members_j2c +
                           SRC_JSON2C_AUX_STR.format(type=m['type'],
                                                     name=m['name']))
        func_def = SRC_C2JSON_DEF_STR.format(type=k, klass=klass)
        func_def = indent_func_def(func_def)
        src = src + SRC_C2JSON_STR.format(func_def=func_def, #type=k,
                                          members=members_c2j)
                                          # klass=klass)

        func_def = SRC_JSON2C_DEF_STR.format(type=k, klass=klass)
        func_def = indent_func_def(func_def)

        src = src + SRC_JSON2C_STR.format(func_def=func_def,
                                          members=members_j2c)
    return header, funcs, src


#
def get_methods_header_string(methods):
    """Generates the header function definitions for sw device client APIs"""
    methods_hdr_str = ''
    for m in methods:
        s = '  {ret} {method}('.format(**m)
        len_s = len(s)
        for i, p in enumerate(m['in']):
            padding = len_s * ' ' if i != 0 else ''
            s = s + '{0}const {1} &{2},\n'.format(padding, p[0], p[1])
        padding = len_s * ' ' if m['in'] else ''
        for i, p in enumerate(m['out']):
            s = s + '{0}{1} *{2},\n'.format(padding, p[0], p[1])
        methods_hdr_str = (methods_hdr_str + m['doc'] +
                           (s[:-2] if s[-2] == ',' else s) + ');\n')
    return methods_hdr_str

#
def callbacks_hrd_string(methods):
    """Generates the header function definitions for sw device server
    callback APIs
    """
    callbacks_hdr_str = ''
    for m in methods:
        s = '  virtual {ret} {method}_cb('.format(**m)
        len_s = len(s)
        for i, p in enumerate(m['in']):
            padding = len_s * ' ' if i != 0 else ''
            s = s + '{0}const {1} &{2},\n'.format(padding, p[0], p[1])
        padding = len_s * ' ' if m['in'] else ''
        for i, p in enumerate(m['out']):
            s = s + '{0}{1} *{2},\n'.format(padding, p[0], p[1])
        callbacks_hdr_str = (callbacks_hdr_str + m['doc'] +
                             (s[:-2] if s[-2] == ',' else s) + ');\n')
    return callbacks_hdr_str

#
def callbacks_priv_hdr_string(methods):
    """Generates the header function definitions for sw device server private
    callback APIs
    """
    callbacks_priv_hdr_str = ''
    for m in methods:
        callbacks_priv_hdr_str = (
            callbacks_priv_hdr_str +
            '  {ret} {method}_cb_p(void *param, void *result);\n'.format(**m))
    return callbacks_priv_hdr_str

#
MULTI_PARAM_STR = """  // create a list with all parameters
  nl::json jset;
{param}  void *jset_ptr = reinterpret_cast<void*>(&jset);
"""

MULTI_PARAM_ITEM_STR = """  jset.push_back(j{name});
"""

METHOD_JSON2C_SRC_STR = """  if ({name} != NULL) {{
    err = {type}_json2c(jget_ptr, {name});
  }}"""

METHOD_C2JSON_SRC_STR = """  nl::json j{name};
  void *j{name}_ptr = reinterpret_cast<void*>(&j{name});
  if (err = {type}_c2json({name}, j{name}_ptr)) {{
    return err;
  }}
"""

METHOD_SRC_STR = """
{sig}) {{
  uint64_t err = 0LL;

  // parse parameters
{c2json}{param}
  // send command
  nl::json jget;
  void *jget_ptr = reinterpret_cast<void*>(&jget);

  unsigned int timeout = {timeout};
  if (err = SendRawMsg("{method}", jset_ptr, timeout, jget_ptr)) {{
    return err;
  }}
  // get the results
{json2c}
  return err;
}}
"""

def methods_src_string(methods):
    """Generates the source code for sw device client APIs"""
    methods_src_str = ''
    for m in methods:
        s = '{ret} {class}::{method}('.format(**m)
        len_s = len(s)
        src_c2json = ''
        src_json2c = ''
        param = ''
        for i, p in enumerate(m['in']):
            padding = len_s*' ' if i != 0 else ''
            s = s + '{0}const {1} &{2},\n'.format(padding,
                                                  p[0],
                                                  p[1])
            src_c2json = src_c2json + METHOD_C2JSON_SRC_STR.format(type=p[0],
                                                                   name=p[1])
            param = param + MULTI_PARAM_ITEM_STR.format(name=p[1])
        #
        for i, p in enumerate(m['out']):
            padding = len_s * ' ' if m['in'] else ''
            s = s + '{0}{1} *{2},\n'.format(padding,
                                            p[0],
                                            p[1])
            src_json2c = src_json2c + METHOD_JSON2C_SRC_STR.format(type=p[0],
                                                                   name=p[1])

        param = MULTI_PARAM_STR.format(param=param)
        methods_src_str = (
            methods_src_str +
            METHOD_SRC_STR.format(sig=s[:-2] if s[-2] == ',' else s,
                                  c2json=src_c2json,
                                  json2c=src_json2c,
                                  method=m['method'],
                                  timeout=m['timeout'],
                                  param=param))
    return methods_src_str

#
PROCESS_CMD_STR = """
uint64_t {klass}::ProcessCommand(const char *method, void *param, void *result) {{
  typedef uint64_t ({klass}::*cb)(void*, void*);

  std::map<std::string, cb> map = {{
{methods}  }};
  return (this->*(map[method]))(param, result);
}}
"""

def process_cmd_string(klass, swdev_api):
    """Generates the ProcessCommand API that is used to route the method string
    in the jsonrpc to the private server callback
    """
    methods_str = ''
    for m in swdev_api:
        methods_str = (methods_str +
                       "    {{\"{m}\", &{klass}::{m}_cb_p}},\n".format(
                           m=m['method'],
                           klass=klass))
    process_cmd = PROCESS_CMD_STR.format(klass=klass,
                                         methods=methods_str,
                                         num_methods=len(swdev_api)-1)
    return process_cmd

#
CALLBACK_SRC_STR = """{sig}) {{
  return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_FUNC_NOT_AVAILABLE);
}}
"""

def callback_string(methods):
    """Generates the default callback methods. These methods should be overriden
    in a derived class to provide the actual sw device server functionality
    """
    callback_str = ''
    for m in methods:
        s = '{ret} {class}::{method}_cb('.format(**m)
        len_s = len(s)
        src_c2json = ''
        src_json2c = ''
        param = ''
        for i, p in enumerate(m['in']):
            padding = len_s*' ' if i != 0 else ''
            s = s + '{0}const {1} &,\n'.format(padding,
                                               p[0])
            src_c2json = src_c2json + METHOD_C2JSON_SRC_STR.format(type=p[0],
                                                                   name=p[1])
            param = param + MULTI_PARAM_ITEM_STR.format(name=p[1])
        #
        padding = len_s * ' ' if len(m['in']) != 0 else ''
        for i, p in enumerate(m['out']):
            s = s + '{0}{1} *,\n'.format(padding,
                                         p[0])
            src_json2c = src_json2c + METHOD_JSON2C_SRC_STR.format(type=p[0],
                                                                   name=p[1])
        callback_str = callback_str + CALLBACK_SRC_STR.format(
            sig=s[:-2] if s[-2] == ',' else s) + '\n'
    return callback_str

#
# callback_priv_string
#
CALLBACK_JSON2C_STR = """  {type} {name};
  try {{
    if (err = {type}_json2c(&params->at({idx}), &{name})) {{
      return err;
    }}
  }} catch (nl::json::exception) {{     // out_of_range or type_error
    return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_INVALID_PARAM);
  }}
"""

CALLBACK_C2JSON_STR = """
  if (err = {type}_c2json({name}, result)) {{
    return err;
  }}
"""

CALLBACK_PRIV_STR = """uint64_t {klass}::{method}_cb_p(void *param, void *result) {{
  uint64_t err = 0LL;
  nl::json *params = reinterpret_cast<nl::json*>(param);

{callback_j2c}{callback_call_def}
  if (err = {callback_call}) {{
    return err;
  }}{callback_c2j}  return err;
}}

"""

def callback_priv_string(klass, methods):
    """Generates the callback private methods. These methods will convert the
    jsonrpc to c types, call the (overriden by the user) callback and
    convert the return values back to jsonrcp so they can be sent to SoHal
    """
    callback_priv = ''
    for m in methods:
        callback_j2c = ''
        callback_call_def = ''
        callback_call = '{}_cb('.format(m['method'])
        callback_c2j = ''
        for i, p in enumerate(m['in']):
            callback_j2c += CALLBACK_JSON2C_STR.format(type=p[0],
                                                       name=p[1],
                                                       idx=i)
            callback_call += '{}, '.format(p[1])

        for i, p in enumerate(m['out']):
            callback_c2j += CALLBACK_C2JSON_STR.format(type=p[0], name=p[1])
            callback_call += '&{}, '.format(p[1])
            callback_call_def += '  {type} {name};'.format(
                type=p[0], name=p[1])

        callback_call = (callback_call[:-2]
                         if callback_call[-2] == ','
                         else callback_call) + ')'

        callback_priv += (
            CALLBACK_PRIV_STR.format(klass=klass,
                                     method=m['method'],
                                     callback_j2c=callback_j2c,
                                     callback_call_def=callback_call_def,
                                     callback_call=callback_call,
                                     callback_c2j=callback_c2j))
    return callback_priv

#
def copyright_str(swdev):
    """Generates the copyright message"""
    return reduce('{}\n// {}\n'.format, swdev['copyright'])

#
def get_type_conversions(klass, ptype, path, all_types):
    """Generates type definitions (in header), json2c and c2json conversion
    function definition (in funcs) and source code (in src) for the types
    passed as argument
    """
    schema_file_name = os.path.join(path,
                                    'schemas',
                                    ptype + '.schema.json')
    structs = {}
    depth = 0
    parse_schema(structs, depth, schema_file_name, all_types)
    return generate_header_funcs_src(klass, structs)

#
def header_funcs_src(klass, param_types, path):
    """Generates type definitions (in header), json2c and c2json conversion
    function definition (in funcs) and source code (in src) for all types
    defined in the schema
    """
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
    return header, funcs, src

#
def get_header(cright, klass, methods, header, funcs):
    """Generates the sw device header (.h) file"""
    # public methods header string
    methods_hdr_str = get_methods_header_string(methods)
    # public callbacks
    callbacks_hdr_str = callbacks_hrd_string(methods)
    # protected callbacks
    callbacks_priv_hdr_str = callbacks_priv_hdr_string(methods)
    #
    return HEADER_STR.format(copyright=cright,
                             KLASS=klass.upper(),
                             klass=klass,
                             methods=methods_hdr_str,
                             structs=header,
                             callbacks=callbacks_hdr_str,
                             callbacks_priv=callbacks_priv_hdr_str,
                             parsers=funcs)

#
def get_source(cright, klass, swdev, methods, src):
    """Generates the sw device source code (.cc) file"""
    # public methods src string
    methods_src_str = methods_src_string(methods)
    # process cmd
    process_cmd = process_cmd_string(klass, swdev['api'])
    # callback
    callback_str = callback_string(methods)
    # callback priv
    callback_priv = callback_priv_string(klass, methods)
    #
    sw_json = get_sw_json(swdev)
    #
    return SOURCE_STR.format(copyright=cright,
                             klass=klass,
                             name=klass.lower(),
                             methods=methods_src_str,
                             process_cmd=process_cmd,
                             callbacks=callback_str,
                             callbacks_priv=callback_priv,
                             json=sw_json,
                             parsers=src)

###########

# pylint: disable=too-many-locals
def main():
    """Main function"""
    parser = argparse.ArgumentParser()
    parser.add_argument("--json",
                        help='json file with SW device description')
    parser.add_argument("--schemas",
                        help='path to find the schemas files')
    args = parser.parse_args()
    # generate the cpp files
    swdev = get_swdev(args.json, args.schemas)
    klass = swdev['klass']
    # public methods
    methods, param_types = get_methods(klass, swdev['api'])
    # main
    path = os.path.dirname(os.path.realpath(__file__))
    cright = copyright_str(swdev)
    #
    header, funcs, src = header_funcs_src(klass, param_types, path)
    #
    # putting together the header file
    #
    all_header = get_header(cright, klass, methods, header, funcs)
    header_file = os.path.join(path,
                               'include',
                               '{}.h'.format(klass.lower()))
    os.makedirs(os.path.dirname(header_file), exist_ok=True)
    with open(header_file, 'w') as fp:
        fp.write(all_header)
    #
    # putting together the source file
    #
    all_src = get_source(cright, klass, swdev, methods, src)
    source_file = os.path.join(path,
                               'src',
                               '{}.cc'.format(klass.lower()))
    with open(source_file, 'w') as fp:
        fp.write(all_src)

#
if __name__ == '__main__':
    main()
