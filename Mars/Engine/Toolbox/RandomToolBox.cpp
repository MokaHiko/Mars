#include "RandomToolBox.h"

tbx::PrimeSearch::PrimeSearch(int n_elements)
	: _n_elements(n_elements)
{
	assert(n_elements > 0 && "Cant' do prime search if no elements, buddy boy!");

	_a = _random_engine.Next();
	_b = _random_engine.Next();
	_c = _random_engine.Next();

	_skip = (_a * (_n_elements * _n_elements)) + (_b * _n_elements) + _c;
	_skip &= ~0xc0000000;

	_prime = FindNextPrime(n_elements);

	// Prevent collapse to 0
	int test = (_skip % _prime);
	if (!test)
	{
		_skip++;
	}
}

int tbx::PrimeSearch::GetNext(bool restart)
{
	if (restart)
	{
		Restart();
	}

	if (_searches == _prime)
	{
		return -1;
	}

	_nextMember += _skip;
	_nextMember %= _prime;
	_searches++;

	return _nextMember < _n_elements ? _nextMember : GetNext();
}

void tbx::PrimeSearch::Restart()
{
	_nextMember = 0;
	_searches = 0;
}

int tbx::PrimeSearch::FindNextPrime(int n)
{
	// Base case
	if (n <= 1)
	{
		return 2;
	}

	bool found = false;
	while (!found)
	{
		n++;
		if (IsPrime(n))
		{
			return n;
		}
	}

	return 0;
}

int tbx::PrimeSearch::IsPrime(int n)
{
	// Corner cases
	if (n <= 1)
	{
		return false;
	}
	if (n <= 3)
	{
		return false;
	}

	if (n % 2 == 0 || n % 3 == 0)
	{
		return false;
	}

	for (int i = 5; i * i <= n; i = i + 6)
	{
		if (n % i == 0 || n % (i + 2) == 0)
		{
			return false;
		}
	}

	return true;
}
