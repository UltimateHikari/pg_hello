#include "postgres.h"

#include "fmgr.h"
#include "utils/builtins.h"
#include "utils/guc.h"
#include "executor/executor.h"
#include "postmaster/bgworker.h"
#include "miscadmin.h"
#include "postmaster/interrupt.h"
#include "tcop/tcopprot.h"

char *message = "hello, cworld!";
static bool hello_logs = true;
static ExecutorStart_hook_type prev_ExecutorStart = NULL;

PG_MODULE_MAGIC;

PG_FUNCTION_INFO_V1(hello_cworld);
static void hello_ExecutorStart(QueryDesc *queryDesc, int eflags);
static void hello_start_worker(void);
PGDLLEXPORT void hello_main(Datum main_arg);

Datum
hello_cworld(PG_FUNCTION_ARGS)
{
	PG_RETURN_TEXT_P(cstring_to_text(message));
}

void
_PG_init(void)
{
	DefineCustomBoolVariable("pg_hello.enabled",
							 "Enable sample logs on hook called.",
							 NULL,
							 &hello_logs,
							 true,
							 PGC_POSTMASTER,
							 0,
							 NULL,
							 NULL,
							 NULL);

	prev_ExecutorStart = ExecutorStart_hook;
	ExecutorStart_hook = hello_ExecutorStart;

	hello_start_worker();
}


static void
hello_ExecutorStart(QueryDesc *queryDesc, int eflags)
{
	if (prev_ExecutorStart)
		prev_ExecutorStart(queryDesc, eflags);
	else
		standard_ExecutorStart(queryDesc, eflags);

	if (hello_logs)
		ereport(LOG, (errmsg("hello_hook: start executing query"),
					  errdetail("query: %s", queryDesc->sourceText)));
}

static void
hello_start_worker(void)
{
	BackgroundWorker worker;

	MemSet(&worker, 0, sizeof(BackgroundWorker));
	worker.bgw_flags = BGWORKER_SHMEM_ACCESS;
	worker.bgw_start_time = BgWorkerStart_PostmasterStart;
	strcpy(worker.bgw_library_name, "pg_hello");
	strcpy(worker.bgw_function_name, "hello_main");
	strcpy(worker.bgw_name, "pg_hello healthcheck");
	strcpy(worker.bgw_type, "pg_hello healthcheck");

	RegisterBackgroundWorker(&worker);
}

void
hello_main(Datum main_arg)
{
	//pqsignal(SIGTERM, SignalHandlerForShutdownRequest);
	pqsignal(SIGTERM, die);
	pqsignal(SIGHUP, SignalHandlerForConfigReload);
	BackgroundWorkerUnblockSignals();

	//while (!ShutdownRequestPending)
	while (true)
	{
		CHECK_FOR_INTERRUPTS();

		ereport(LOG, errmsg("pg_hello_health_check"));

		sleep(3);
	}
}