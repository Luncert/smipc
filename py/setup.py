from setuptools import setup

# TODO: path

setup(
    name='smipc',
    version='1.0.0',
    py_modules=['smipc'],
    author='Luncert',
    author_email='2725115515@qq.com',
    url='https://github.com/Luncert/smipc',
    description='IPC lib based on shared memory.',
    data_files=[('lib', ['../core/src/cmake-build-debug/libsmipc.dll'])],
    keywords=['ipc', 'shared memory']
)
