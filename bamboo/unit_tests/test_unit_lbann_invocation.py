import sys
sys.path.insert(0, '../common_python')
import tools
import os, sys


# Run with python3 -m pytest -s test_unit_lbann_invocation.py -k 'test_unit_no_params_bad' --exes=<executable>
def test_unit_no_params_bad(cluster, exes):
    if isinstance(exes, dict):
        exe = exes['gcc7']
    else:
        exe = exes
    sys.stderr.write('TESTING: run lbann with no params; lbann should throw exception\n')
    command = tools.get_command(
        cluster=cluster, executable=exe, exit_after_setup=True)
    return_code = os.system(command)
    assert return_code != 0


# Run with python3 -m pytest -s test_unit_lbann_invocation.py -k 'test_unit_one_model_bad' --exes=<executable>
def test_unit_one_model_bad(cluster, exes):
    if isinstance(exes, dict):
        exe = exes['gcc7']
    else:
        exe = exes
    sys.stderr.write('TESTING: run lbann with no optimizer or reader; lbann should throw exception\n')
    model_path = 'prototext/model_mnist_simple_1.prototext'
    command = tools.get_command(
        cluster=cluster, executable=exe, exit_after_setup=True,
        model_path=model_path)
    return_code = os.system(command)
    assert return_code != 0


# Run with python3 -m pytest -s test_unit_lbann_invocation.py -k 'test_unit_two_models_bad' --exes=<executable>
def test_unit_two_models_bad(cluster, exes):
    if isinstance(exes, dict):
        exe = exes['gcc7']
    else:
        exe = exes
    sys.stderr.write('TESTING: run lbann with two models but no optimizer or reader; lbann should throw exception\n')
    model_path = '{prototext/model_mnist_simple_1.prototext,prototext/model_mnist_simple_1.prototext}'
    command = tools.get_command(
        cluster=cluster, executable=exe, exit_after_setup=True,
        model_path=model_path)
    return_code = os.system(command)
    assert return_code != 0


# Run with python3 -m pytest -s test_unit_lbann_invocation.py -k 'test_unit_two_models_bad2' --exes=<executable>
def test_unit_two_models_bad2(cluster, exes):
    if isinstance(exes, dict):
        exe = exes['gcc7']
    else:
        exe = exes
    sys.stderr.write('TESTING: run lbann with two models with missing {; lbann should throw exception\n')
    model_path='prototext/model_mnist_simple_1.prototext,prototext/model_mnist_simple_1.prototext}'
    command = tools.get_command(
        cluster=cluster, executable=exe, exit_after_setup=True,
        model_path=model_path)
    return_code = os.system(command)
    assert return_code != 0


# Run with python3 -m pytest -s test_unit_lbann_invocation.py -k 'test_unit_missing_optimizer' --exes=<executable>
def test_unit_missing_optimizer(cluster, exes):
    if isinstance(exes, dict):
        exe = exes['gcc7']
    else:
        exe = exes
    sys.stderr.write('TESTING: run lbann with two models, reader, but no optimizer; lbann should throw exception\n')
    model_path='{prototext/model_mnist_simple_1.prototext,prototext/model_mnist_simple_1.prototext}'
    data_reader_path='prototext/data_reader_mnist.prototext'
    command = tools.get_command(
        cluster=cluster, executable=exe, data_reader_path=data_reader_path,
        data_filedir_default='/p/lscratchh/brainusr/datasets/MNIST',
        exit_after_setup=True, model_path=model_path)
    return_code = os.system(command)
    assert return_code != 0


# Run with python3 -m pytest -s test_unit_lbann_invocation.py -k 'test_unit_missing_reader' --exes=<executable>
def test_unit_missing_reader(cluster, exes):
    if isinstance(exes, dict):
        exe = exes['gcc7']
    else:
        exe = exes
    sys.stderr.write('TESTING: run lbann with two models, reader, but no reader; lbann should throw exception\n')
    model_path = '{prototext/model_mnist_simple_1.prototext,prototext/model_mnist_simple_1.prototext}'
    optimizer_path = 'prototext/opt_sgd.prototext'
    command = tools.get_command(
        cluster=cluster, executable=exe, exit_after_setup=True,
        model_path=model_path, optimizer_path=optimizer_path)
    return_code = os.system(command)
    assert return_code != 0


# Run with python3 -m pytest -s test_unit_lbann_invocation.py -k 'test_unit_bad_params' --exes=<executable>
def test_unit_bad_params(cluster, exes):
    if isinstance(exes, dict):
        exe = exes['gcc7']
    else:
        exe = exes
    sys.stderr.write('TESTING: run lbann with ill-formed param (missing -) lbann should throw exception\n')
    (command_allocate, command_run, _, _) = tools.get_command(cluster=cluster, executable=exe, return_tuple=True)
    command_string = '%s%s %s -exit_after_setup --reader=prototext/data_reader_mnist.prototext --model={prototext/model_mnist_simple_1.prototext,prototext/model_mnist_simple_1.prototext} --optimizer=prototext/opt_sgd.prototext' % (command_allocate, command_run, exe)
    return_code = os.system(command_string)
    assert return_code != 0


# Run with python3 -m pytest -s test_unit_lbann_invocation.py -k 'test_unit_should_work' --exes=<executable>
def test_unit_should_work(cluster, exes):
    if isinstance(exes, dict):
        exe = exes['gcc7']
    else:
        exe = exes
    sys.stderr.write('TESTING: run lbann with two models, reader, and optimizer; lbann should NOT throw exception\n')
    model_path = '{prototext/model_mnist_simple_1.prototext,prototext/model_mnist_simple_1.prototext}'
    data_reader_path = 'prototext/data_reader_mnist.prototext'
    optimizer_path = 'prototext/opt_sgd.prototext'
    command = tools.get_command(
        cluster=cluster, executable=exe, data_reader_path=data_reader_path,
        data_filedir_default='/p/lscratchh/brainusr/datasets/MNIST',
        exit_after_setup=True, model_path=model_path,
        optimizer_path=optimizer_path)
    return_code = os.system(command)
    assert return_code != 0
