#服务器IP地址
Server_IP=172.18.0.104

#用户名
User_Name=fengxiaopei

#服务器密码
Password=fengxiaopei

echo "*************ssh server!!!*******************************"

./bin/expect_c $Server_IP $User_Name $Password

echo "*************ssh server exit!*******************************"


