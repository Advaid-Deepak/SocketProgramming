all:
	g++ -o client-phase1 client-phase1.cpp -lpthread 
	g++ -o client-phase2 client-phase2.cpp -lpthread
	g++ -o client-phase3 client-phase3.cpp -lpthread
	g++ -o client-phase4 client-phase4.cpp -lpthread
	g++ -o client-phase5 client-phase5.cpp -lpthread -lcrypto
