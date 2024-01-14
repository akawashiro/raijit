# raijit

## CPython 3.12 reading memo
- Python/generated_cases.c.h に各オペコードの実行がある

## Other JIT Compiler
- https://github.com/tonybaloney/Pyjion
- https://github.com/google-deepmind/s6
    - https://github.com/google-deepmind/s6#richards


## TODO
- Run [pyperformance](https://github.com/python/pyperformance).

## Run tests
```bash
./run_primitive_tests.py
```

## MEMO
- [JITコンパイルをサポートした新たなRuby処理系をRustとアセンブリで書いている話](https://qiita.com/sisshiki1969/items/c4ab0d61f1e88f1ad99e)
- [RJIT: RubyでRubyのJITコンパイラを書いた](https://k0kubun.hatenablog.com/entry/rjit)
- [RustでJITコンパイルする電卓を実装してみる](https://qiita.com/0yoyoyo/items/b809f0c24347d65ea885)
- [RubyKaigiでJITコンパイラの書き方について発表した](https://k0kubun.hatenablog.com/entry/2023/05/18/171546)
- [RustでJITコンパイルする電卓を実装してみる](https://qiita.com/0yoyoyo/items/b809f0c24347d65ea885)
- [スタックのアラインメント](https://gist.github.com/msymt/4d708d079a66aa45a3e26a1f7587c172)
- [x86-64 モードのプログラミングではスタックのアライメントに気を付けよう](https://uchan.hateblo.jp/entry/2018/02/16/232029)
- [LOAD_GLOBAL implementation](https://github.com/python/cpython/blob/6c2f34fa77f884bd79801a9ab8a117cab7d9c7ed/Python/ceval.c#L2958)
- [System V AMD64 ABI 呼出規約](https://ja.wikipedia.org/wiki/%E5%91%BC%E5%87%BA%E8%A6%8F%E7%B4%84#System_V_AMD64_ABI_.E5.91.BC.E5.87.BA.E8.A6.8F.E7.B4.84)
- [Python Opcodes](https://unpyc.sourceforge.net/Opcodes.html)
- [Brandt Bucher – A JIT Compiler for CPython](https://www.youtube.com/watch?v=HxSHIpEQRjs)
    - [Slides](https://github.com/brandtbucher/brandtbucher/blob/master/2023/10/10/a_jit_compiler_for_cpython.pdf)
- [opv86](https://hikalium.github.io/opv86/)
- [JITあれこれ](https://keens.github.io/blog/2018/12/01/jitarekore/)
- [YARV Maniacs 【第 3 回】 命令ディスパッチの高速化](https://magazine.rubyist.net/articles/0008/0008-YarvManiacs.html)
- [JIT コンパイラのコンパイラスレッド、コード最適化について](https://rabbitfoot141.hatenablog.com/entry/2020/12/02/020945)
