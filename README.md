 

编译步骤：
  安装依赖
	1.ompl
	2.eband_local_planner
  编译fuzzylite库
    1.cd src/fuzzylite/fuzzylite
	2.mkdir build && cd build
	3.cmake ..
	4.make
	5.sudo make install
	

底盘驱动相关：
	线速度：twist.linear.x,这里的线速度范围为500～2500（对应PWM脉冲为0.5ms～2.5ms）,1500为静止，1500-2500为正向速度，500-1500为反向速度。  
	角速度：twist.angular.x，这里角度范围为0～180度,90度为中间值，90-180度左转，0-90度右转。  

	

	
