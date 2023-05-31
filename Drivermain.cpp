#include "global.h"
#include "SetContextUserCall.h"
#include <ntifs.h>
#include <ntddk.h>

PVOID NtBase = 0;
int _fltused = 0;

VOID DriverUnload(DRIVER_OBJECT* DriverObject)
{

}

#define TEST CTL_CODE(FILE_DEVICE_UNKNOWN, 0x999, METHOD_OUT_DIRECT, FILE_ANY_ACCESS)


UNICODE_STRING g_DeviceName = RTL_CONSTANT_STRING(L"\\Device\\APC_Drv");

UNICODE_STRING g_SymbolicLink = RTL_CONSTANT_STRING(L"\\??\\APC_Drv");


struct DataStruct
{
	HANDLE PID;
};

NTSTATUS CreateFunction(PDEVICE_OBJECT device, PIRP irp)
{
	irp->IoStatus.Status = STATUS_SUCCESS;
	irp->IoStatus.Information = 0;

	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}


NTSTATUS ControlFunction(PDEVICE_OBJECT device, PIRP irp)
{
	PIO_STACK_LOCATION Stack = IoGetCurrentIrpStackLocation(irp);

	ULONG Code = Stack->Parameters.DeviceIoControl.IoControlCode;

	struct DataStruct Data;

	NTSTATUS Status = STATUS_SUCCESS;
	DbgPrint("[+] Control");
	switch (Code)
	{
	case TEST:
	{
		memcpy(&Data, irp->AssociatedIrp.SystemBuffer, sizeof(Data));
		DbgPrint("[+} Load Function\n");
		DbgPrint("[+] Pid: %d\n", Data.PID);
		CtxCall.Call(Data.PID);
		break;
	}
	}

	irp->IoStatus.Status = STATUS_SUCCESS;
	irp->IoStatus.Information = 0;
	IoCompleteRequest(irp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}

NTSTATUS DriverEntry(PDRIVER_OBJECT DrvObj, PVOID reg)
{
	//DrvObj->DriverUnload = DriverUnload;

	getKernelModuleByName("ntoskrnl.exe", &::NtBase);


	PDEVICE_OBJECT Device = NULL;

	IoCreateDevice(DrvObj, 0, &g_DeviceName, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &Device);
	IoCreateSymbolicLink(&g_SymbolicLink, &g_DeviceName);


	DrvObj->MajorFunction[IRP_MJ_CREATE] = CreateFunction;
	DrvObj->MajorFunction[IRP_MJ_DEVICE_CONTROL] = ControlFunction;
	__dbgdb();
	DbgPrint("[+} Load");

	return 0;
}


