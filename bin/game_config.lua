--lua格式配置文件 <明星97配置文件>

quanpan_rate = {0,   20,  40, 60, 110,80, 210,400,30,  1000,550}	--全盘机率
quanpan_bet =  {2000,1000,700,500,300,400,200,100,1000,15,  30}	--全盘倍率

seven_mul = {0,2,5,10,20,50,100,200,2000} --7的个数对应倍率

grid_rate = {		--每个格子的机率
    {3,5,7,9,20,15,25,13,3},
    {3,5,7,9,15,20,25,10,6},
    {3,5,7,9,20,20,20,10,6},
    {3,5,7,9,10,15,25,20,6},
    {3,3,5,7,10,8,12,12,40},
    {3,5,7,9,8,6,10,12,40},
    {3,5,7,9,25,11,15,20,5},
    {3,5,7,9,8,6,10,12,40},
    {3,5,7,9,8,6,10,12,40}
}
for i=1,#grid_rate do	--配置检查
    local sum=0
    for j=1,9 do sum=grid_rate[i][j]+sum end
    assert(sum==100,"配错了!!!机率不满100")
end
