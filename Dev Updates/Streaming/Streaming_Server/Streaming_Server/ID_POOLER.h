#pragma once
#include <mutex>
#include <queue>


class ID_POOLER
{
private:
	std::mutex id_lock;
	std::queue<int> id_pool;
	int current_user;
	int max_user;

public:
	ID_POOLER(int MAX_USER);
	~ID_POOLER();

	int GetNewClientId();
	void DeleteClientId(int id);
};