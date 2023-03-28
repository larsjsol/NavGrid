#include "NavGridPlugin.h"


void NavGridPluginImpl::StartupModule()
{
	UE_LOG(NavGrid, Log, TEXT("Starting"));
}

void NavGridPluginImpl::ShutdownModule()
{
	UE_LOG(NavGrid, Log, TEXT("Shutting down"));
}

IMPLEMENT_MODULE(NavGridPluginImpl, NavGrid)
