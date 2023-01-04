

sequential_server:
	gcc server_sequence.cpp -o server_seq && ./server_seq

pthread_server:
	gcc server_pthread.cpp -o server_pthread && ./server_pthread

fork_server:
	gcc server_fork.cpp -o server_fork && ./server_fork

select_server:
	gcc server_select.cpp -o server_select && ./server_select

poll_server:
	gcc server_poll.cpp -o server_poll && ./server_poll

epoll_server:
	gcc server_epoll.cpp -o server_epoll && ./server_epoll

sequential_client:
	gcc client.cpp -o client
	./client

concurrent_client:
	gcc client_thread.cpp -o client_thread
	./client_thread

time_server:
	gcc client_thread.cpp -o client_thread
	time ./client_thread

compile_all_server:
	gcc server_seq.cpp -o server_seq
	gcc server_fork.cpp -o server_fork
	gcc server_select.cpp -o server_select
	gcc server_poll.cpp -o server_poll
	gcc server_epoll.cpp -o server_epoll

cleanup:
	rm -f -- server_fork server_pthread client client_thread clientt epollf server_seq serverf serverp servers server_epoll server_poll server_select