#1 docker build -t game:v1.0.0 ./
	NEED LIST:
		Dockerfile
		game
		game_start.sh
		random_first.txt
		random_last.txt
#2(opt)docker images
#3(opt)docker run --rm -d -P game:v1.0.0
#4(opt)docker ps -a

#--------------------------------docker hub-------------------------------------#
#1docker pull xiazheng/game:v1.0.0
#2docker tag xiazheng/game:v1.0.0 game:v1.0.0
#3(opt) docker images
#4(opt)docker run --rm -d -P game:v1.0.0
#5(opt)docker ps -a

