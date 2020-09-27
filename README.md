# 国际空间站任务SpaceNode软件
Version 1.0

## 背景

该工程为SpaceChain在国际空间站节点任务中运行的载荷软件，用于批量处理SpaceChain多重签名安全支付交易。

## 硬件介绍

硬件采用GomSpace NanoMind Z7000:
* Xilinx XC7Z030 平台
* dual-core ARM Cortex-A9
* Kintex-7 FPGA
* 1 GB DDR3 memory
* 64 MB QSPI NOR flash
* 4 GB eMMC flash 
* I2C、CAN、Ethernet

## 软件介绍

SpaceNode软件运行在SpaceChain ISS节点上，主要功能:
* 地面站认证交换
* 在轨秘钥生成与下发
* 批量SpaceChain多签交易处理
* 卫星公钥更新下发
* 企业用户秘钥更新下发

## 编译

* 执行make命令
* 在bin目录下生成spacenode可执行文件


