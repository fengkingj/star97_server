-- lua格式脚本 明星97游戏规则

grid_name = { "seven","blue","red","green","bell","watermelon","grape","orange","cherry" }--所有图案
fruit_grid = { "watermelon","grape","orange","cherry" }--水果
bar_grid = { "blue","red","green" }--bar

quanpan_bet =  {2000,1000,700,500,300,400,200,100,1000,15,  30}--全盘倍率
quanpan_rate = {0,   20,  40, 60, 110,80, 210,400,30,  1000,550}--全盘机率

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
{3,5,7,9,8,6,10,12,40}}
for i=1,9 do	--检查一下
  local sum=0
  for j=1,9 do sum=grid_rate[i][j]+sum end
  assert(sum==100,"配错了!!! 机率不满100")
end

award_line = { {1,2,3},{4,5,6},{7,8,9},{1,4,7},{2,5,8},{3,6,9},{1,5,9},{7,5,3} }--连线定义

award_group = {	--中奖组合和其倍率
{ "seven","seven","seven" ,100},
{ "bell","bell","bell" ,18},
{ "watermelon","watermelon","watermelon" ,20},
{ "grape","grape","grape" ,14},
{ "orange","orange","orange" ,10},
{ "blue","blue","blue" ,70},
{ "blue","blue","red" ,10},
{ "blue","blue","green" ,10},
{ "blue","red","red" ,10},
{ "blue","red","green" ,10},
{ "blue","red","blue" ,10},
{ "blue","green","green" ,10},
{ "blue","green","red" ,10},
{ "blue","green","blue" ,10},
{ "red","red","red" ,50},
{ "red","red","blue" ,10},
{ "red","red","green" ,10},
{ "red","blue","blue" ,10},
{ "red","blue","green" ,10},
{ "red","blue","red" ,10},
{ "red","green","green" ,10},
{ "red","green","blue" ,10},
{ "red","green","red" ,10},
{ "green","green","green" ,35},
{ "green","green","red" ,10},
{ "green","green","blue" ,10},
{ "green","red","red" ,10},
{ "green","red","blue" ,10},
{ "green","red","green" ,10},
{ "green","blue","blue" ,10},
{ "green","blue","red" ,10},
{ "green","blue","green" ,10}}
