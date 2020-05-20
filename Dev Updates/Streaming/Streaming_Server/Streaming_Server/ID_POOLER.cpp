#include "ID_POOLER.h"

ID_POOLER::ID_POOLER(int MAX_USER) :
	current_user(0),
	max_user(MAX_USER)
{
	for (int i = 0; i < max_user; ++i)
		id_pool.emplace(i);
}

ID_POOLER::~ID_POOLER()
{
}

int ID_POOLER::GetNewClientId()
{
	if (current_user >= max_user) return -1;

	id_lock.lock();
	++current_user;
	int new_id = id_pool.front();
	id_pool.pop();
	id_lock.unlock();
	return new_id;
}

void ID_POOLER::DeleteClientId(int id)
{
	id_lock.lock();
	--current_user;
	id_pool.push(id);
	id_lock.unlock();
}
