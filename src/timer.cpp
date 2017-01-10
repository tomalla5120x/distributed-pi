#include "timer.h"

#include <stdexcept>
#include <signal.h>

Timer::Timer(int signal, int timeout, bool oneshot): m_bOneShot(oneshot), m_signal(signal)
{
	sigevent sev;
	sev.sigev_notify = SIGEV_SIGNAL;
	sev.sigev_signo = signal;
	
	if(timer_create(CLOCK_REALTIME, &sev, &m_timer) == -1)
		throw std::runtime_error("Error creating timer");
	
	m_nSec = timeout / 1000;
	m_nNsec = (timeout % 1000) * 1000000;
}

Timer::~Timer()
{
	timer_delete(m_timer);
}

void Timer::set()
{
	itimerspec its;
	
	its.it_value.tv_sec = m_nSec;
	its.it_value.tv_nsec = m_nNsec;
	
	if(m_bOneShot)
	{
		its.it_interval.tv_sec = 0;
		its.it_interval.tv_nsec = 0;
	} else
	{
		its.it_interval.tv_sec = m_nSec;
		its.it_interval.tv_nsec = m_nNsec;
	}
	
	if(timer_settime(m_timer, 0, &its, NULL) == -1)
		throw std::runtime_error("Error setting timer");
}

void Timer::unset()
{
	itimerspec its;
	
	its.it_value.tv_sec = 0;
	its.it_value.tv_nsec = 0;
	its.it_interval.tv_sec = 0;
	its.it_interval.tv_nsec = 0;
	
	if(timer_settime(m_timer, 0, &its, NULL) == -1)
		throw std::runtime_error("Error unsetting timer");
}

bool Timer::isRunning() const
{
	itimerspec its;
	
	if(timer_gettime(m_timer, &its) == -1)
		throw std::runtime_error("Error unsetting timer");

	return (its.it_value.tv_sec != 0 || its.it_value.tv_nsec != 0);
}