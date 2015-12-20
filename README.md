## netdump  

[![Build Status](https://travis-ci.org/leeqx/netdump.png)](https://travis-ci.org/leeqx/netdump)  
[![Build Status](https://codeship.com/projects/1c5edb80-888d-0133-a64f-2eae657aa6d0/status?branch=master)](https://codeship.com/projects/)  
[![Circle CI](https://circleci.com/gh/leeqx/netdump.svg?style=svg)](https://circleci.com/gh/leeqx/netdump)   
抓包工具，可以实现自己的数据解析插件、抓取数据包然后转换成相应的格式输出

eg：
 mydump -s 0 -S 127.0.0.1 -D 192.168.1.1 -P 80 -c 10 -i lo

