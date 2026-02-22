import shutil
import pathlib

scriptDir = pathlib.Path(__file__).parent
cpythonSrcDir = scriptDir.parent / "cpython"
genCPythonSrcDir = scriptDir / "cpython"

changedFiles = [
    "Grammar/Grammar",
    "Include/token.h",
    "Modules/parsermodule.c",
    "Parser/tokenizer.c",
    "Python/ast.c",
    "Python/graminit.c",
    "Python/compile.c",
    "Python/Python-ast.c",
    "Include/Python-ast.h",
    "Include/graminit.h",
    "Python/symtable.c",
]

if genCPythonSrcDir.exists():
    shutil.rmtree(genCPythonSrcDir)
    print(f"已删除旧的生成目录: {genCPythonSrcDir}")

print("正在生成修改后的cpython源代码...")
for file in changedFiles:
    srcFile = cpythonSrcDir / file
    dstFile = genCPythonSrcDir / file
    # 检查目标src文件是否存在
    if not srcFile.exists():
        print(f"源文件 {srcFile} 不存在，跳过")
        continue
    print(f"正在处理 {srcFile} -> {dstFile}")
    dstFile.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(srcFile, dstFile)

print("生成完成！")