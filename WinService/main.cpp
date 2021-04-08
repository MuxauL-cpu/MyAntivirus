#include "Service.h"
#include <tchar.h>
#include <locale.h>

int _tmain(int argc,TCHAR* argv[])
{
	setlocale(LC_ALL, "Russian");
	Service* svc = new Service;
	Service::setServiceInstance(svc);
	Service::ServiceProcess(argc,argv);
	delete svc;
	return 0;
}