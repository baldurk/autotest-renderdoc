RenderDoc Autotesting
==============

This is a suite of tiny programs and control logic for auto-testing RenderDoc on small features and bugs, to make sure nothing has regressed.

Tests are added to stress specific features, entry points, pieces of functionality etc. Some are written specifically, some are written after the fact to verify a bugfix made to renderdoc. They test both capture and replay (ie. can we correctly replay the results of an application) as well as analysis functionality (can we debug a shader correctly, display the correct pipeline state, etc).

The main repository is [renderdoc](https://github.com/baldurk/renderdoc), go there for more information!

Building
--------------

I assume VS 2013 and latest Win 8.1 SDK, unlike renderdoc I won't be supporting old VS or using the DX SDK. This is mostly to keep things simpler - I'll need to test D3D11.1 or D3D11.2 features. The VS 2013 support is for string literals to more easily embed shaders in source, and sadly VS 2010 doesn't support that C++11 feature.

License
--------------

RenderDoc is released under the MIT license, see [LICENSE.md](LICENSE.md) for full text. This repository is under the same license.

The tests use [GLEW](http://glew.sourceforge.net/) for extension loading, which is BSD licensed.

The harness that runs tests and interfaces with renderdoc's API to run tests and check things uses [duktape](http://duktape.org/) which is MIT licensed
