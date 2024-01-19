SAG 平台主仿真脚本（NS3）使用说明

安装：
1. 安装NS3，推荐在ubuntu或虚拟机ubuntu系统上安装，其它系统安装容易出现问题。 参考流程： https://www.nsnam.org/wiki/Installation （推荐手动方式，只下载先不要执行build）
2. 在执行 Building ns-3 with build.py 之前，下载SNS3软件包并放置到ns3相应文件夹中。参考流程： https://github.com/sns3/sns3-satellite （推荐waf方式，这样可以和NS3一起编译）
   备注： 从git上下载三个包解压后，文件夹名称需要修改。 sns3-satellite改为satellite, sns3-stats改为magister-stats， sns3-traffic改为traffic。 不然编译会报错
3. 参考NS3安装教程编译和使用waf配置NS3及SNS3。
4. 如安装过程中出现各种问题无法解决，可以尝试直接导入平台所在的虚拟机。不过虚拟机比较大（10G+），需要足够大U盘或带宽传输。 

基本使用：
1. 将SAG主仿真脚本文件 spaceairground_nov.cc 放到NS3主程序scratch文件夹下
2. 在NS3主程序新建swsdefine文件夹 （把压缩包内swsdefine文件夹放到NS3主程序即可）
3. 进入scratch文件夹，打开命令行terminal
4. 输入 sudo ./waf --run scratch/spaceairground_nov 2>&1 | tee outputfilename 即可在屏幕上查看输出结果，以及保存到outputfile中。

如有任何问题，随时联系 时伟森 w46shi@uwaterloo.ca








