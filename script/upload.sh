#服务器IP地址
Server_IP=172.18.0.104

#用户名
User_Name=fengxiaopei

#服务器密码
Password="fengxiaopei"

#可执行文件名
Binary_name=common_service

#本地文件夹目录，$HOME 表示用户根目录
Local_Folder=$HOME/workspaceline/line_common_service

#服务器文件夹目录，普通用户只可以把文件上传到用户目录下
#例：/home/User_Name/exmple 若服务器目录下存在example将会把本地文件夹存入作为example,
#若不存在example文件夹则新建一个example,若想覆盖服务器的example文件夹，则此地应该输入/home/User_Name/
#如果你想下载为一个重命令的文件夹/home/User_Name/exmple2，下载后命名为exmple2
Server_Location=/home/fengxiaopei/workspaceline/line_common_service

find ../src -name "*.o"  | xargs rm -f""

echo "*************updoing script begin!*******************************"
echo "**功能：将本地文件夹，清除*～文件、make clean、上传至服务器、make、运行**"

echo "copy file to destination"

echo "connect and copy folder to server!"

./bin/expect_scp $Server_IP $User_Name $Password $Local_Folder/src $Server_Location/

sleep 0.5

echo "sshing to server , make clean ,make !"

./bin/expect_ssh3 $Server_IP $User_Name $Password $Server_Location $Binary_name

sleep 0.5
rm $HOME/desktop/bin_line/$Binary_name
echo "down bin to local!"
./bin/expect_down $Server_IP $User_Name $Password $Server_Location/bin/$Binary_name $HOME/desktop/bin_line/


Server_IP=47.90.36.42
User_Name=gcgame
Password='56e5fb0ec219745a'
To_Location=/home/gcgame/server/fish_bin
echo "up bin to server!"
./bin/expect_upfile $Server_IP $User_Name $Password $HOME/desktop/bin_line/$Binary_name $To_Location/

sleep 0.5

./bin/expect_c $Server_IP $User_Name $Password



echo "*************upmake script exit!*******************************"


