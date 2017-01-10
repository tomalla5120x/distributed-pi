#ifndef TIMER_H
#define TIMER_H

#include <time.h>

class Timer
{	
public:
	
	// tworzy obiekt Timera.
	// * signal - numer sygnału, który ma zostać generowany przy wygasaniu Timera
	// * timeout - okres Timera (w milisekundach)
	// * oneshot - czy po wygaśnięciu Timer ma ponownie zacząć odliczać czas
	// rzuca wyjątek runtime_error, jeżeli wystąpi błąd
	Timer(int signal, int timeout, bool oneshot);
	~Timer();
	
	// startuje Timer lub resetuje go, jeżeli już działa
	// rzuca wyjątek runtime_error, jeżeli wystąpi błąd
	void set();
	
	// zatrzymuje Timer
	// rzuca wyjątek runtime_error, jeżeli wystąpi błąd
	void unset();
	
	// czy timer jest aktualnie uruchomiony, czy też nie
	// rzuca wyjątek runtime_error, jeżeli wystąpi błąd
	bool isRunning() const;
	
private:
	bool m_bOneShot;
	
	timer_t m_timer;
	int m_nSec;
	int m_nNsec;
	int m_signal;
};

#endif