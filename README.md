# YSYX-miniEMU

这是一生一芯 v24.07 E6 阶段的 miniEMU 实现

## 目录结构

目录结构基于[lcthw](https://docs.kilvn.com/lcthw-zh/)的c-skeleton.  
`src`: 存放miniEMU的主要逻辑代码和debug工具代码.  
`tests`: 存放测试代码.  

## 项目运行

首先获取测试所需的文件:  

```bash
wget https://ysyx.oscc.cc/slides/resources/archive/logisim-bin.tar.bz2
```

然后解压:  

```bash
tar -xvf logisim-bin.tar.bz2 && rm logisim-bin.tar.bz2
```

之后将测试所需的文件从hex转为bin文件:  

```bash
cd logisim-bin
tail -n +2 sum.hex | sed -e 's/.*: //' -e 's/ /\n/g' | sed -e 's/\(..\)\(..\)\(..\)\(..\)/\4 \3 \2 \1/' | xxd -r -p > sum.bin
tail -n +2 mem.hex | sed -e 's/.*: //' -e 's/ /\n/g' | sed -e 's/\(..\)\(..\)\(..\)\(..\)/\4 \3 \2 \1/' | xxd -r -p > mem.bin
```

最后在项目根目录运行编译：  

```bash
make
```

测试日志存放于`tests\tests.log`.

