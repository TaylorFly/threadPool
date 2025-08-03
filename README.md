## 流程分析
+ 每个task都有一个result
+ 每个result也有一个task
+ 当submit一个task的时候,返回一个result
+ 这个result与一个task互相绑定
+ threadFunc执行这个task的时候会将run方法返回的结果作为result的值
+ 