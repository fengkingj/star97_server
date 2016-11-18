#服务器IP地址
Server_IP=172.18.0.56

#用户名
User_Name=gcgame

#服务器密码
Password='game123(*&'

echo "*************ssh server!!!*******************************"

./bin/expect_c $Server_IP $User_Name $Password

echo "*************ssh server exit!*******************************"


