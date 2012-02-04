
test:
	(	echo 1111111111111111 ;\
		sleep 0.5 ;\
		echo 2222222222222222 ;\
		sleep 0.5 ;\
		echo 3333333333333333 ;\
		sleep 0.5 ;\
		echo 4444 ;\
	) | ../frotate --size 10 -n 3 --prefix frotate.log
	[ "3333333333333333" = `cat frotate.log.1` ]
	[ "2222222222222222" = `cat frotate.log.2` ]
	[ "4444" = `cat frotate.log` ]
	@echo Test OK.
