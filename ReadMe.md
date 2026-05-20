

### 代码编译教程

1. 首先下载qt5相关的库，下载完后存放到和KDevelop-Train目录同级目录下，最后解压，解压方式选择解压到当前文件
操作完毕后, KDevelop-Train、thirdparty\_install两个在同级位置。

   下载链接: https://365.kdocs.cn/l/crFHGQuSLP9Q



1. 打开cmd窗口,进入当前位置, 创建build目录，进入后执行相关命令。【KDevelop-Train、thirdparty\_install、build三个在同级目录】

   &#x20;   ```
    mkdir build
    cd build
    ```

2. 使用VS中的编译环境,实际请根据自己安装的VS路径来修改

   &#x20;   ```
    call "C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Professional\\VC\\Auxiliary\\Build\\vcvarsamd64\_x86.bat"
    ```

3. 执行相关构建命令，根据Debug或Release自己选择
* 构建Debug命令

  &#x20;   ```
    cmake  -G "Visual Studio 16 2019"  -A x64 -DCMAKE\_BUILD\_TYPE=Debug  ../KDevelop-Training

    msbuild /m KDevelop-Train.sln /p:Platform=x64 /p:Configuration=Debug
    ```



* 构建Release命令

  &#x20;   ```
    cmake  -G "Visual Studio 16 2019"  -A x64 -DCMAKE\_BUILD\_TYPE=Release  ../KDevelop-Training

    msbuild /m KDevelop-Train.sln /p:Platform=x64 /p:Configuration=Release

