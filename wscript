# -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

# def options(opt):
#     pass

# def configure(conf):
#     conf.check_nonfatal(header_name='stdint.h', define_name='HAVE_STDINT_H')

def build(bld):
    module = bld.create_ns3_module('dfs-routing-module', ['core'])
    module.source = [
        'model/dfs-routing-module.cc',
        'helper/dfs-routing-module-helper.cc',
        ]

    module_test = bld.create_ns3_module_test_library('dfs-routing-module')
    module_test.source = [
        'test/dfs-routing-module-test-suite.cc',
        ]

    headers = bld(features='ns3header')
    headers.module = 'dfs-routing-module'
    headers.source = [
        'model/dfs-routing-module.h',
        'helper/dfs-routing-module-helper.h',
        ]

   