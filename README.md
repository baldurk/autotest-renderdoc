RenderDoc Autotesting
==============

Instructions pending, this is a work-in-progress repo that will soon be committed to [renderdoc](https://github.com/baldurk/renderdoc).

Quick and dirty getting stated instructions:

1. Build demos projects
2. Install python 3.6 to match the renderdoc built python modules
3. Run `python3 run_tests.py --pyrenderdoc /path/to/renderdoc/x64/Development/pymodules --renderdoc /path/to/renderdoc/x64/Development` or other paths for x86/release builds.
4. The tests will run and output the report in artifacts/
5. Other parameters are available for tuning which tests run etc.

Building
--------------

Build demos.sln in VS 2015, or else demos/CMakeLists.txt on linux.

License
--------------

RenderDoc is released under the MIT license, see [LICENSE.md](LICENSE.md) for full text. This repository is under the same license.

The tests use [GLAD](https://github.com/Dav1dde/glad) for extension loading, which is MIT licensed. [LZ4](https://github.com/lz4/lz4) for compression, which is BSD licensed. [volk](https://github.com/zeux/volk) for vulkan loading, which is MIT licensed. [nuklear](https://github.com/vurtun/nuklear) for the launcher UI, which is MIT licensed. [shaderc](https://github.com/google/shaderc) for building SPIR-V shaders, which is Apache-2.0 licensed.
