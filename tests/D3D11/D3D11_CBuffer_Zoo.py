import rdtest
import renderdoc as rd


class D3D11_CBuffer_Zoo(rdtest.TestCase):
    platform = 'win32'

    def get_capture(self):
        return rdtest.run_and_capture("demos_x64", "D3D11_CBuffer_Zoo", 5)

    def check_capture(self):
        draw = self.find_draw("Draw")

        self.check(draw is not None)

        self.controller.SetFrameEvent(draw.eventId, False)

        pipe: rd.PipeState = self.controller.GetPipelineState()

        stage = rd.ShaderStage.Pixel
        cbuf: rd.BoundCBuffer = pipe.GetConstantBuffer(stage, 0, 0)

        var_check = rdtest.ConstantBufferChecker(
            self.controller.GetCBufferVariableContents(pipe.GetShader(stage),
                                                       pipe.GetShaderEntryPoint(stage), 0,
                                                       cbuf.resourceId, cbuf.byteOffset))

        # For more detailed reference for the below checks, see the commented definition of the cbuffer
        # in the shader source code in the demo itself

        # float4 a;
        var_check.check('a').rows(1).cols(4).value([0.0, 1.0, 2.0, 3.0])

        # float3 b;
        var_check.check('b').rows(1).cols(3).value([4.0, 5.0, 6.0])

        # float2 c; float2 d;
        var_check.check('c').rows(1).cols(2).value([8.0, 9.0])
        var_check.check('d').rows(1).cols(2).value([10.0, 11.0])

        # float e; float3 f;
        var_check.check('e').rows(1).cols(1).value([12.0])
        var_check.check('f').rows(1).cols(3).value([13.0, 14.0, 15.0])

        # float g; float2 h; float i;
        var_check.check('g').rows(1).cols(1).value([16.0])
        var_check.check('h').rows(1).cols(2).value([17.0, 18.0])
        var_check.check('i').rows(1).cols(1).value([19.0])

        # float j; float2 k;
        var_check.check('j').rows(1).cols(1).value([20.0])
        var_check.check('k').rows(1).cols(2).value([21.0, 22.0])

        # float2 l; float m;
        var_check.check('l').rows(1).cols(2).value([24.0, 25.0])
        var_check.check('m').rows(1).cols(1).value([26.0])

        # float n[4];
        var_check.check('n').rows(0).cols(0).arraySize(4).members({
            0: lambda x: x.rows(1).cols(1).value([28.0]),
            1: lambda x: x.rows(1).cols(1).value([32.0]),
            2: lambda x: x.rows(1).cols(1).value([36.0]),
            3: lambda x: x.rows(1).cols(1).value([40.0]),
        })

        # float4 dummy1;
        var_check.check('dummy1')

        # float o[4];
        var_check.check('o').rows(0).cols(0).arraySize(4).members({
            0: lambda x: x.rows(1).cols(1).value([48.0]),
            1: lambda x: x.rows(1).cols(1).value([52.0]),
            2: lambda x: x.rows(1).cols(1).value([56.0]),
            3: lambda x: x.rows(1).cols(1).value([60.0]),
        })

        # float p;
        var_check.check('p').rows(1).cols(1).value([61.0])

        # float4 dummy2;
        var_check.check('dummy2')

        # float4 dummygl1;
        # float4 dummygl2;
        var_check.check('dummygl1')
        var_check.check('dummygl2')

        # column_major float4x4 q;
        var_check.check('q').rows(4).cols(4).column_major().value([76.0, 80.0, 84.0, 88.0,
                                                                   77.0, 81.0, 85.0, 89.0,
                                                                   78.0, 82.0, 86.0, 90.0,
                                                                   79.0, 83.0, 87.0, 91.0])

        # row_major float4x4 r;
        var_check.check('r').rows(4).cols(4).row_major().value([92.0, 93.0, 94.0, 95.0,
                                                                96.0, 97.0, 98.0, 99.0,
                                                                100.0, 101.0, 102.0, 103.0,
                                                                104.0, 105.0, 106.0, 107.0])

        # column_major float3x4 s;
        var_check.check('s').rows(3).cols(4).column_major().value([108.0, 112.0, 116.0, 120.0,
                                                                   109.0, 113.0, 117.0, 121.0,
                                                                   110.0, 114.0, 118.0, 122.0])

        # float4 dummy3;
        var_check.check('dummy3')

        # row_major float3x4 t;
        var_check.check('t').rows(3).cols(4).row_major().value([128.0, 129.0, 130.0, 131.0,
                                                                132.0, 133.0, 134.0, 135.0,
                                                                136.0, 137.0, 138.0, 139.0])

        # float4 dummy4;
        var_check.check('dummy4')

        # column_major float2x3 u;
        var_check.check('u').rows(2).cols(3).column_major().value([144.0, 148.0, 152.0,
                                                                   145.0, 149.0, 153.0])

        # float4 dummy5;
        var_check.check('dummy5')

        # row_major float2x3 v;
        var_check.check('v').rows(2).cols(3).row_major().value([160.0, 161.0, 162.0,
                                                                164.0, 165.0, 166.0])

        # float4 dummy6;
        var_check.check('dummy6')

        # column_major float2x2 w;
        var_check.check('w').rows(2).cols(2).column_major().value([172.0, 176.0,
                                                                   173.0, 177.0])

        # float4 dummy7;
        var_check.check('dummy7')

        # row_major float2x2 x;
        var_check.check('x').rows(2).cols(2).row_major().value([184.0, 185.0,
                                                                188.0, 189.0])

        # float4 dummy8;
        var_check.check('dummy8')

        # row_major float2x2 y;
        var_check.check('y').rows(2).cols(2).row_major().value([196.0, 197.0,
                                                                200.0, 201.0])

        # float z;
        var_check.check('z').rows(1).cols(1).value([202.0])

        # float4 gldummy3;
        var_check.check('gldummy3')

        # row_major float4x1 aa;
        var_check.check('aa').rows(4).cols(1).value([208.0, 212.0, 216.0, 220.0])

        # column_major float4x1 ab;
        var_check.check('ab').rows(4).cols(1).value([224.0, 225.0, 226.0, 227.0])

        # float4 multiarray[3][2];
        # this is flattened to just multiarray[6]
        var_check.check('multiarray').rows(0).cols(0).arraySize(6).members({
            0: lambda x: x.rows(1).cols(4).value([228.0, 229.0, 230.0, 231.0]),
            1: lambda x: x.rows(1).cols(4).value([232.0, 233.0, 234.0, 235.0]),
            2: lambda x: x.rows(1).cols(4).value([236.0, 237.0, 238.0, 239.0]),
            3: lambda x: x.rows(1).cols(4).value([240.0, 241.0, 242.0, 243.0]),
            4: lambda x: x.rows(1).cols(4).value([244.0, 245.0, 246.0, 247.0]),
            5: lambda x: x.rows(1).cols(4).value([248.0, 249.0, 250.0, 251.0]),
        })

        # struct float3_1 { float3 a; float b; };
        # struct nested { float3_1 a; float4 b[4]; float3_1 c[4]; };
        # nested structa[2];
        var_check.check('structa').rows(0).cols(0).arraySize(2).members({
            # structa[0]
            0: lambda s: s.rows(0).cols(0).structSize(3).members({
                'a': lambda x: x.rows(0).cols(0).structSize(2).members({
                    'a': lambda y: y.rows(1).cols(3).value([252.0, 253.0, 254.0]),
                    'b': lambda y: y.rows(1).cols(1).value([255.0]),
                }),
                'b': lambda x: x.rows(0).cols(0).arraySize(4).members({
                    0: lambda y: y.rows(1).cols(4).value([256.0, 257.0, 258.0, 259.0]),
                    1: lambda y: y.rows(1).cols(4).value([260.0, 261.0, 262.0, 263.0]),
                    2: lambda y: y.rows(1).cols(4).value([264.0, 265.0, 266.0, 267.0]),
                    3: lambda y: y.rows(1).cols(4).value([268.0, 269.0, 270.0, 271.0]),
                }),
                'c': lambda x: x.rows(0).cols(0).arraySize(4).members({
                    0: lambda y: y.rows(0).cols(0).structSize(2).members({
                        'a': lambda z: z.rows(1).cols(3).value([272.0, 273.0, 274.0]),
                        'b': lambda z: z.rows(1).cols(1).value([275.0]),
                    }),
                    1: lambda y: y.rows(0).cols(0).structSize(2).members({
                        'a': lambda z: z.rows(1).cols(3).value([276.0, 277.0, 278.0]),
                        'b': lambda z: z.rows(1).cols(1).value([279.0]),
                    }),
                    2: lambda y: y.rows(0).cols(0).structSize(2).members({
                        'a': lambda z: z.rows(1).cols(3).value([280.0, 281.0, 282.0]),
                        'b': lambda z: z.rows(1).cols(1).value([283.0]),
                    }),
                    3: lambda y: y.rows(0).cols(0).structSize(2).members({
                        'a': lambda z: z.rows(1).cols(3).value([284.0, 285.0, 286.0]),
                        'b': lambda z: z.rows(1).cols(1).value([287.0]),
                    }),
                }),
            }),
            # structa[1]
            1: lambda s: s.rows(0).cols(0).structSize(3).members({
                'a': lambda x: x.rows(0).cols(0).structSize(2).members({
                    'a': lambda y: y.rows(1).cols(3).value([288.0, 289.0, 290.0]),
                    'b': lambda y: y.rows(1).cols(1).value([291.0]),
                }),
                'b': lambda x: x.rows(0).cols(0).arraySize(4).members({
                    0: lambda y: y.rows(1).cols(4).value([292.0, 293.0, 294.0, 295.0]),
                    1: lambda y: y.rows(1).cols(4).value([296.0, 297.0, 298.0, 299.0]),
                    2: lambda y: y.rows(1).cols(4).value([300.0, 301.0, 302.0, 303.0]),
                    3: lambda y: y.rows(1).cols(4).value([304.0, 305.0, 306.0, 307.0]),
                }),
                'c': lambda x: x.rows(0).cols(0).arraySize(4).members({
                    0: lambda y: y.rows(0).cols(0).structSize(2).members({
                        'a': lambda z: z.rows(1).cols(3).value([308.0, 309.0, 310.0]),
                        'b': lambda z: z.rows(1).cols(1).value([311.0]),
                    }),
                    1: lambda y: y.rows(0).cols(0).structSize(2).members({
                        'a': lambda z: z.rows(1).cols(3).value([312.0, 313.0, 314.0]),
                        'b': lambda z: z.rows(1).cols(1).value([315.0]),
                    }),
                    2: lambda y: y.rows(0).cols(0).structSize(2).members({
                        'a': lambda z: z.rows(1).cols(3).value([316.0, 317.0, 318.0]),
                        'b': lambda z: z.rows(1).cols(1).value([319.0]),
                    }),
                    3: lambda y: y.rows(0).cols(0).structSize(2).members({
                        'a': lambda z: z.rows(1).cols(3).value([320.0, 321.0, 322.0]),
                        'b': lambda z: z.rows(1).cols(1).value([323.0]),
                    }),
                }),
            }),
        })

        # float4 test;
        var_check.check('test').rows(1).cols(4).value([324.0, 325.0, 326.0, 327.0])

        var_check.done()

        rdtest.log.success("CBuffer variables are as expected")
