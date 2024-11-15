/*
 * Copyright (c) 2022 Winsider Seminars & Solutions, Inc.  All rights reserved.
 *
 * This file is part of System Informer.
 *
 * Authors:
 *
 *     wj32         2010-2011
 *     Dart Vanya   2024
 *
 */

#include "exttools.h"
#include <symprv.h>
#include <secedit.h>
#include <hndlinfo.h>
#include <kphuser.h>

 // Columns

#define ETHNLVC_PROCESS 0
#define ETHNLVC_HANDLE 1
#define ETHNLVC_ACCESS 2
#define ETHNLVC_ATTRIBUTES 3

#define ETDTLVC_NAME 0
#define ETDTLVC_SID 1
#define ETDTLVC_HEAP 2
#define ETDTLVC_IO 3

typedef struct _COMMON_PAGE_CONTEXT
{
    PPH_HANDLE_ITEM HandleItem;
    HANDLE ProcessId;
    HWND WindowHandle;
    HWND ListViewHandle;
} COMMON_PAGE_CONTEXT, *PCOMMON_PAGE_CONTEXT;

typedef struct _ET_HANDLE_ENTRY
{
    HANDLE ProcessId;
    PPH_HANDLE_ITEM HandleItem;
    COLORREF Color;
    BOOLEAN OwnHandle;
} HANDLE_ENTRY, * PHANDLE_ENTRY;

HPROPSHEETPAGE EtpCommonCreatePage(
    _In_ PPH_PLUGIN_HANDLE_PROPERTIES_CONTEXT Context,
    _In_ PWSTR Template,
    _In_ DLGPROC DlgProc
    );

INT CALLBACK EtpCommonPropPageProc(
    _In_ HWND hwnd,
    _In_ UINT uMsg,
    _In_ LPPROPSHEETPAGE ppsp
    );

INT_PTR CALLBACK EtpTpWorkerFactoryPageDlgProc(
    _In_ HWND hwndDlg,
    _In_ UINT uMsg,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam
    );

INT_PTR CALLBACK EtpObjHandlesPageDlgProc(
    _In_ HWND hwndDlg,
    _In_ UINT uMsg,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam
    );

PPH_STRING EtGetAccessString(
    _In_ PPH_STRING TypeName,
    _In_ ACCESS_MASK access
    );

PPH_STRING EtGetAccessString2(
    _In_ PPH_STRINGREF TypeName,
    _In_ ACCESS_MASK access
    );

PPH_STRING EtGetAccessStringZ(
    _In_ PCWSTR TypeName,
    _In_ ACCESS_MASK access
    );

INT_PTR CALLBACK EtpWinStaPageDlgProc(
    _In_ HWND hwndDlg,
    _In_ UINT uMsg,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam
    );

VOID EtHandlePropertiesInitializing(
    _In_ PVOID Parameter
    )
{
    PPH_PLUGIN_OBJECT_PROPERTIES objectProperties = Parameter;
    PPH_PLUGIN_HANDLE_PROPERTIES_CONTEXT context = objectProperties->Parameter;

    if (PhIsNullOrEmptyString(context->HandleItem->TypeName))
        return;

    if (objectProperties->NumberOfPages < objectProperties->MaximumNumberOfPages)
    {
        HPROPSHEETPAGE page = NULL;

        if (PhEqualString2(context->HandleItem->TypeName, L"WindowStation", TRUE))
        {
            page = EtpCommonCreatePage(
                context,
                MAKEINTRESOURCE(IDD_OBJWINSTA),
                EtpWinStaPageDlgProc
                );

            // Insert our page into the second slot.
            if (page)
            {
                if (objectProperties->NumberOfPages > 1)
                {
                    memmove(&objectProperties->Pages[2], &objectProperties->Pages[1],
                        (objectProperties->NumberOfPages - 1) * sizeof(HPROPSHEETPAGE));
                }

                objectProperties->Pages[1] = page;
                objectProperties->NumberOfPages++;
            }
        }

        // Object Manager
        if (EtObjectManagerDialogHandle && context->OwnerPlugin == PluginInstance)
        {
            page = EtpCommonCreatePage(
                context,
                MAKEINTRESOURCE(IDD_OBJHANDLES),
                EtpObjHandlesPageDlgProc
                );

            // Insert our page into the second slot.
            if (page)
            {
                if (objectProperties->NumberOfPages > 1)
                {
                    memmove(&objectProperties->Pages[2], &objectProperties->Pages[1],
                        (objectProperties->NumberOfPages - 1) * sizeof(HPROPSHEETPAGE));
                }

                objectProperties->Pages[1] = page;
                objectProperties->NumberOfPages++;
            }
        }

        // TpWorkerFactory unnamed, so it won't interact with Object Manager
        if (PhEqualString2(context->HandleItem->TypeName, L"TpWorkerFactory", TRUE))
        {
            page = EtpCommonCreatePage(
                context,
                MAKEINTRESOURCE(IDD_OBJTPWORKERFACTORY),
                EtpTpWorkerFactoryPageDlgProc
                );

            // Insert our page into the second slot.
            if (page)
            {
                if (objectProperties->NumberOfPages > 1)
                {
                    memmove(&objectProperties->Pages[2], &objectProperties->Pages[1],
                        (objectProperties->NumberOfPages - 1) * sizeof(HPROPSHEETPAGE));
                }

                objectProperties->Pages[1] = page;
                objectProperties->NumberOfPages++;
            }
        }
    }
}

typedef enum _ET_OBJECT_GENERAL_CATEGORY
{
    OBJECT_GENERAL_CATEGORY_DEVICE = PH_PLUGIN_HANDLE_GENERAL_CATEGORY_FILE,
    OBJECT_GENERAL_CATEGORY_DRIVER,
    OBJECT_GENERAL_CATEGORY_TYPE,
    OBJECT_GENERAL_CATEGORY_TYPE_ACCESS,
    OBJECT_GENERAL_CATEGORY_WINDOWSTATION,
    OBJECT_GENERAL_CATEGORY_DESKTOP
} ET_OBJECT_GENERAL_CATEGORY;

typedef enum _ET_OBJECT_GENERAL_INDEX {
    OBJECT_GENERAL_INDEX_ATTRIBUTES = PH_PLUGIN_HANDLE_GENERAL_INDEX_ACCESSMASK,
    OBJECT_GENERAL_INDEX_CREATIONTIME,

    OBJECT_GENERAL_INDEX_DEVICEDRVLOW,
    OBJECT_GENERAL_INDEX_DEVICEDRVLOWPATH,
    OBJECT_GENERAL_INDEX_DEVICEDRVHIGH,
    OBJECT_GENERAL_INDEX_DEVICEDRVHIGHPATH,
    OBJECT_GENERAL_INDEX_DEVICEPNPNAME,

    OBJECT_GENERAL_INDEX_DRIVERIMAGE,
    OBJECT_GENERAL_INDEX_DRIVERSERVICE,
    OBJECT_GENERAL_INDEX_DRIVERSIZE,
    OBJECT_GENERAL_INDEX_DRIVERSTART,
    OBJECT_GENERAL_INDEX_DRIVERFLAGS,

    OBJECT_GENERAL_INDEX_TYPEINDEX,
    OBJECT_GENERAL_INDEX_TYPEOBJECTS,
    OBJECT_GENERAL_INDEX_TYPEHANDLES,
    OBJECT_GENERAL_INDEX_TYPEPEAKOBJECTS,
    OBJECT_GENERAL_INDEX_TYPEPEAKHANDLES,
    OBJECT_GENERAL_INDEX_TYPEPOOLTYPE,
    OBJECT_GENERAL_INDEX_TYPEPAGECHARGE,
    OBJECT_GENERAL_INDEX_TYPENPAGECHARGE,

    OBJECT_GENERAL_INDEX_TYPEVALIDMASK,
    OBJECT_GENERAL_INDEX_TYPEGENERICREAD,
    OBJECT_GENERAL_INDEX_TYPEGENERICWRITE,
    OBJECT_GENERAL_INDEX_TYPEGENERICEXECUTE,
    OBJECT_GENERAL_INDEX_TYPEGENERICALL,
    OBJECT_GENERAL_INDEX_TYPEINVALIDATTRIBUTES,

    OBJECT_GENERAL_INDEX_WINSTATYPE,
    OBJECT_GENERAL_INDEX_WINSTAVISIBLE,

    OBJECT_GENERAL_INDEX_DESKTOPIO,
    OBJECT_GENERAL_INDEX_DESKTOPSID,
    OBJECT_GENERAL_INDEX_DESKTOPHEAP,

    OBJECT_GENERAL_INDEX_MAXIMUM
} ET_OBJECT_GENERAL_INDEX;

typedef enum _ET_OBJECT_POOLTYPE {
    PagedPool = 1,
    NonPagedPool = 0,
    NonPagedPoolNx = 0x200,
    NonPagedPoolSessionNx = NonPagedPoolNx + 32,
    PagedPoolSessionNx = NonPagedPoolNx + 33
} ET_OBJECT_POOLTYPE;

#define OBJECT_CHILD_HANDLEPROP_WINDOW 1

VOID EtHandlePropertiesWindowInitialized(
    _In_ PVOID Parameter
    )
{
    static INT EtListViewRowCache[OBJECT_GENERAL_INDEX_MAXIMUM];

    PPH_PLUGIN_HANDLE_PROPERTIES_WINDOW_CONTEXT context = Parameter;
    WCHAR string[PH_INT64_STR_LEN_1];

    if (EtObjectManagerDialogHandle && context->OwnerPlugin == PluginInstance)
    {
        if (context->HandleItem->Handle && context->ProcessId == NtCurrentProcessId())
        {
            PhReferenceObject(EtObjectManagerPropWindows);
            PhAddItemSimpleHashtable(EtObjectManagerPropWindows, context->ParentWindow, context->HandleItem);
        }

        // HACK
        if (PhGetIntegerPairSetting(SETTING_NAME_OBJMGR_PROPERTIES_WINDOW_POSITION).X != 0)
            PhLoadWindowPlacementFromSetting(SETTING_NAME_OBJMGR_PROPERTIES_WINDOW_POSITION, NULL, context->ParentWindow);
        else
            PhCenterWindow(context->ParentWindow, GetParent(context->ParentWindow)); // HACK

        PhSetWindowIcon(context->ParentWindow, EtObjectManagerPropIcon, NULL, 0);

        // Show real handles count
        ULONG64 real_count;
        PPH_STRING count = PH_AUTO(PhGetListViewItemText(context->ListViewHandle, PH_PLUGIN_HANDLE_GENERAL_INDEX_HANDLES, 1));
       
        if (!PhIsNullOrEmptyString(count) && PhStringToUInt64(&count->sr, 0, &real_count) && real_count > 0) {
            ULONG own_count = 0;
            PPH_KEY_VALUE_PAIR entry;
            ULONG i = 0;

            while (PhEnumHashtable(EtObjectManagerPropWindows, &entry, &i))
                if (PhEqualString(context->HandleItem->ObjectName, ((PPH_HANDLE_ITEM)entry->Value)->ObjectName, TRUE))
                    own_count++;

            PhPrintUInt32(string, (ULONG)real_count - own_count);
            PhSetListViewSubItem(context->ListViewHandle, PH_PLUGIN_HANDLE_GENERAL_INDEX_HANDLES, 1, string);
        }

        PhRemoveListViewItem(context->ListViewHandle, PH_PLUGIN_HANDLE_GENERAL_INDEX_ACCESSMASK);

        EtListViewRowCache[OBJECT_GENERAL_INDEX_ATTRIBUTES] = PhAddListViewGroupItem(
            context->ListViewHandle,
            PH_PLUGIN_HANDLE_GENERAL_CATEGORY_BASICINFO,
            OBJECT_GENERAL_INDEX_ATTRIBUTES,
            L"Object attributes",
            NULL
            );

        // Show object attributes
        PPH_STRING Attributes;
        PH_STRING_BUILDER stringBuilder;

        PhInitializeStringBuilder(&stringBuilder, 10);
        if (context->HandleItem->Attributes & OBJ_PERMANENT)
            PhAppendStringBuilder2(&stringBuilder, L"Permanent, ");
        if (context->HandleItem->Attributes & OBJ_EXCLUSIVE)
            PhAppendStringBuilder2(&stringBuilder, L"Exclusive, ");
        if (context->HandleItem->Attributes & OBJ_KERNEL_HANDLE)
            PhAppendStringBuilder2(&stringBuilder, L"Kernel object, ");
        if (context->HandleItem->Attributes & PH_OBJ_KERNEL_ACCESS_ONLY)
            PhAppendStringBuilder2(&stringBuilder, L"Kernel only access, ");

        // Remove the trailing ", ".
        if (PhEndsWithString2(stringBuilder.String, L", ", FALSE))
            PhRemoveEndStringBuilder(&stringBuilder, 2);

        Attributes = PH_AUTO(PhFinalStringBuilderString(&stringBuilder));
        PhSetListViewSubItem(context->ListViewHandle, EtListViewRowCache[OBJECT_GENERAL_INDEX_ATTRIBUTES], 1, PhGetString(Attributes));

        // Show creation time
        if (EtObjectManagerTimeCached.QuadPart != 0)
        {
            PPH_STRING startTimeString;
            SYSTEMTIME startTimeFields;

            EtListViewRowCache[OBJECT_GENERAL_INDEX_CREATIONTIME] = PhAddListViewGroupItem(
                context->ListViewHandle,
                PH_PLUGIN_HANDLE_GENERAL_CATEGORY_BASICINFO,
                OBJECT_GENERAL_INDEX_CREATIONTIME,
                L"Creation time",
                NULL
                );

            PhLargeIntegerToLocalSystemTime(&startTimeFields, &EtObjectManagerTimeCached);
            startTimeString = PhaFormatDateTime(&startTimeFields);

            PhSetListViewSubItem(context->ListViewHandle, EtListViewRowCache[OBJECT_GENERAL_INDEX_CREATIONTIME], 1, startTimeString->Buffer);
        }

        // Remove irrelevant information if we couldn't open real object
        if (PhEqualString2(context->HandleItem->TypeName, L"ALPC Port", TRUE))
        {
            PhRemoveListViewItem(context->ListViewHandle, context->ListViewRowCache[PH_PLUGIN_HANDLE_GENERAL_INDEX_ALPCCLIENT]);
            PhRemoveListViewItem(context->ListViewHandle, context->ListViewRowCache[PH_PLUGIN_HANDLE_GENERAL_INDEX_ALPCSERVER]);
            
            if (!context->HandleItem->Object)
            {
                PhSetListViewSubItem(context->ListViewHandle, context->ListViewRowCache[PH_PLUGIN_HANDLE_GENERAL_INDEX_FLAGS], 1, NULL);
                PhSetListViewSubItem(context->ListViewHandle, context->ListViewRowCache[PH_PLUGIN_HANDLE_GENERAL_INDEX_SEQUENCENUMBER], 1, NULL);
                PhSetListViewSubItem(context->ListViewHandle, context->ListViewRowCache[PH_PLUGIN_HANDLE_GENERAL_INDEX_PORTCONTEXT], 1, NULL);
            }
        }
    }
    else if (((ULONG_PTR)context->OwnerPlugin == ((ULONG_PTR)PluginInstance | OBJECT_CHILD_HANDLEPROP_WINDOW)) &&
        !PhIsNullOrEmptyString(context->HandleItem->ObjectName) &&
        !PhEqualString(context->HandleItem->ObjectName, context->HandleItem->BestObjectName, TRUE))
    {
        // I don't know why new row always appending in the back. This stupid full rebuild of section only one workaround
        PPH_STRING accessString = PH_AUTO(PhGetListViewItemText(context->ListViewHandle, context->ListViewRowCache[PH_PLUGIN_HANDLE_GENERAL_INDEX_ACCESSMASK], 1));
        PhRemoveListViewItem(context->ListViewHandle, context->ListViewRowCache[PH_PLUGIN_HANDLE_GENERAL_INDEX_ACCESSMASK]);
        PhRemoveListViewItem(context->ListViewHandle, context->ListViewRowCache[PH_PLUGIN_HANDLE_GENERAL_INDEX_OBJECT]);
        PhRemoveListViewItem(context->ListViewHandle, context->ListViewRowCache[PH_PLUGIN_HANDLE_GENERAL_INDEX_TYPE]);
        PhRemoveListViewItem(context->ListViewHandle, context->ListViewRowCache[PH_PLUGIN_HANDLE_GENERAL_INDEX_NAME]);
        
        context->ListViewRowCache[PH_PLUGIN_HANDLE_GENERAL_INDEX_NAME] = PhAddListViewGroupItem(context->ListViewHandle,
            PH_PLUGIN_HANDLE_GENERAL_CATEGORY_BASICINFO, 0, L"Name", NULL);
        INT origNameIndex = PhAddListViewGroupItem(context->ListViewHandle,
            PH_PLUGIN_HANDLE_GENERAL_CATEGORY_BASICINFO, 1, L"Original name", NULL);
        context->ListViewRowCache[PH_PLUGIN_HANDLE_GENERAL_INDEX_TYPE] = PhAddListViewGroupItem(context->ListViewHandle,
            PH_PLUGIN_HANDLE_GENERAL_CATEGORY_BASICINFO, 2, L"Type", NULL);
        context->ListViewRowCache[PH_PLUGIN_HANDLE_GENERAL_INDEX_OBJECT] = PhAddListViewGroupItem(context->ListViewHandle,
            PH_PLUGIN_HANDLE_GENERAL_CATEGORY_BASICINFO, 3, L"Object address", NULL);
        context->ListViewRowCache[PH_PLUGIN_HANDLE_GENERAL_INDEX_ACCESSMASK] = PhAddListViewGroupItem(context->ListViewHandle,
            PH_PLUGIN_HANDLE_GENERAL_CATEGORY_BASICINFO, 4, L"Granted access", NULL);

        PhSetListViewSubItem(context->ListViewHandle, context->ListViewRowCache[PH_PLUGIN_HANDLE_GENERAL_INDEX_NAME], 1, PhGetString(context->HandleItem->BestObjectName));
        PhSetListViewSubItem(context->ListViewHandle, origNameIndex, 1, PhGetString(context->HandleItem->ObjectName));
        PhSetListViewSubItem(context->ListViewHandle, context->ListViewRowCache[PH_PLUGIN_HANDLE_GENERAL_INDEX_TYPE], 1, PhGetString(context->HandleItem->TypeName));
        PhSetListViewSubItem(context->ListViewHandle, context->ListViewRowCache[PH_PLUGIN_HANDLE_GENERAL_INDEX_OBJECT], 1, context->HandleItem->ObjectString);
        PhSetListViewSubItem(context->ListViewHandle, context->ListViewRowCache[PH_PLUGIN_HANDLE_GENERAL_INDEX_ACCESSMASK], 1, PhGetString(accessString));
    }

    // General ET plugin extensions
    if (!PhIsNullOrEmptyString(context->HandleItem->TypeName))
    {
        // Show Driver image information
        if (PhEqualString2(context->HandleItem->TypeName, L"Driver", TRUE))
        {
            PPH_STRING driverName;
            KPH_DRIVER_BASIC_INFORMATION basicInfo;

            PhAddListViewGroup(context->ListViewHandle, OBJECT_GENERAL_CATEGORY_DRIVER, L"Driver information");

            EtListViewRowCache[OBJECT_GENERAL_INDEX_DRIVERIMAGE] = PhAddListViewGroupItem(context->ListViewHandle,
                OBJECT_GENERAL_CATEGORY_DRIVER, OBJECT_GENERAL_INDEX_DRIVERIMAGE, L"Driver Image", NULL);
            EtListViewRowCache[OBJECT_GENERAL_INDEX_DRIVERSERVICE] = PhAddListViewGroupItem(context->ListViewHandle,
                OBJECT_GENERAL_CATEGORY_DRIVER, OBJECT_GENERAL_INDEX_DRIVERSERVICE, L"Driver Service Name", NULL);
            EtListViewRowCache[OBJECT_GENERAL_INDEX_DRIVERSIZE] = PhAddListViewGroupItem(context->ListViewHandle,
                OBJECT_GENERAL_CATEGORY_DRIVER, OBJECT_GENERAL_INDEX_DRIVERSIZE, L"Driver Size", NULL);
            EtListViewRowCache[OBJECT_GENERAL_INDEX_DRIVERSTART] = PhAddListViewGroupItem(context->ListViewHandle,
                OBJECT_GENERAL_CATEGORY_DRIVER, OBJECT_GENERAL_INDEX_DRIVERSTART, L"Driver Start Address", NULL);
            EtListViewRowCache[OBJECT_GENERAL_INDEX_DRIVERFLAGS] = PhAddListViewGroupItem(context->ListViewHandle,
                OBJECT_GENERAL_CATEGORY_DRIVER, OBJECT_GENERAL_INDEX_DRIVERFLAGS, L"Driver Flags", NULL);

            if (KsiLevel() == KphLevelMax)
            {
                if (NT_SUCCESS(PhGetDriverImageFileName(context->HandleItem->Handle, &driverName)))
                {
                    PhSetListViewSubItem(context->ListViewHandle, EtListViewRowCache[OBJECT_GENERAL_INDEX_DRIVERIMAGE], 1, PhGetString(driverName));
                    PhDereferenceObject(driverName);
                }

                if (NT_SUCCESS(PhGetDriverServiceKeyName(context->HandleItem->Handle, &driverName)))
                {
                    PhSetListViewSubItem(context->ListViewHandle, EtListViewRowCache[OBJECT_GENERAL_INDEX_DRIVERSERVICE], 1, PhGetString(driverName));
                    PhDereferenceObject(driverName);
                }

                if (NT_SUCCESS(KphQueryInformationDriver(
                    context->HandleItem->Handle,
                    KphDriverBasicInformation,
                    &basicInfo,
                    sizeof(KPH_DRIVER_BASIC_INFORMATION),
                    NULL
                    )))
                {
                    PPH_STRING size = PH_AUTO(PhFormatSize(basicInfo.DriverSize, ULONG_MAX));
                    PhSetListViewSubItem(context->ListViewHandle, EtListViewRowCache[OBJECT_GENERAL_INDEX_DRIVERSIZE], 1, PhGetString(size));

                    PhPrintPointer(string, basicInfo.DriverStart);
                    PhSetListViewSubItem(context->ListViewHandle, EtListViewRowCache[OBJECT_GENERAL_INDEX_DRIVERSTART], 1, string);

                    PhPrintPointer(string, ULongToPtr(basicInfo.Flags));
                    PhSetListViewSubItem(context->ListViewHandle, EtListViewRowCache[OBJECT_GENERAL_INDEX_DRIVERFLAGS], 1, string);
                }
            }
        }
        // Show Device drivers information
        else if (PhEqualString2(context->HandleItem->TypeName, L"Device", TRUE))
        {
            HANDLE deviceObject;
            HANDLE deviceBaseObject;
            HANDLE driverObject;
            PPH_STRING driverName;
            OBJECT_ATTRIBUTES objectAttributes;
            UNICODE_STRING objectName;

            PhAddListViewGroup(context->ListViewHandle, OBJECT_GENERAL_CATEGORY_DEVICE, L"Device driver information");

            EtListViewRowCache[OBJECT_GENERAL_INDEX_DEVICEDRVLOW] = PhAddListViewGroupItem(context->ListViewHandle,
                OBJECT_GENERAL_CATEGORY_DEVICE, OBJECT_GENERAL_INDEX_DEVICEDRVLOW, L"Lower-edge driver", NULL);
            EtListViewRowCache[OBJECT_GENERAL_INDEX_DEVICEDRVLOWPATH] = PhAddListViewGroupItem(context->ListViewHandle,
                OBJECT_GENERAL_CATEGORY_DEVICE, OBJECT_GENERAL_INDEX_DEVICEDRVLOWPATH, L"Lower-edge driver image", NULL);

            EtListViewRowCache[OBJECT_GENERAL_INDEX_DEVICEDRVHIGH] = PhAddListViewGroupItem(context->ListViewHandle,
                OBJECT_GENERAL_CATEGORY_DEVICE, OBJECT_GENERAL_INDEX_DEVICEDRVHIGH, L"Upper-edge driver", NULL);
            EtListViewRowCache[OBJECT_GENERAL_INDEX_DEVICEDRVHIGHPATH] = PhAddListViewGroupItem(context->ListViewHandle,
                OBJECT_GENERAL_CATEGORY_DEVICE, OBJECT_GENERAL_INDEX_DEVICEDRVHIGHPATH, L"Upper-edge driver Image", NULL);

            EtListViewRowCache[OBJECT_GENERAL_INDEX_DEVICEPNPNAME] = PhAddListViewGroupItem(context->ListViewHandle,
                OBJECT_GENERAL_CATEGORY_DEVICE, OBJECT_GENERAL_INDEX_DEVICEPNPNAME, L"PnP Device Name", NULL);

            if (KsiLevel() == KphLevelMax)
            {
                PhStringRefToUnicodeString(&context->HandleItem->ObjectName->sr, &objectName);
                InitializeObjectAttributes(&objectAttributes, &objectName, OBJ_CASE_INSENSITIVE, NULL, NULL);

                if (NT_SUCCESS(KphOpenDevice(&deviceObject, READ_CONTROL, &objectAttributes)))
                {
                    if (NT_SUCCESS(KphOpenDeviceDriver(deviceObject, READ_CONTROL, &driverObject)))
                    {
                        if (NT_SUCCESS(PhGetDriverName(driverObject, &driverName)))
                        {
                            PhSetListViewSubItem(context->ListViewHandle, EtListViewRowCache[OBJECT_GENERAL_INDEX_DEVICEDRVHIGH], 1, PhGetString(driverName));
                            PhDereferenceObject(driverName);
                        }

                        if (NT_SUCCESS(PhGetDriverImageFileName(driverObject, &driverName)))
                        {
                            PhSetListViewSubItem(context->ListViewHandle, EtListViewRowCache[OBJECT_GENERAL_INDEX_DEVICEDRVHIGHPATH], 1, PhGetString(driverName));
                            PhDereferenceObject(driverName);
                        }

                        NtClose(driverObject);
                    }

                    if (NT_SUCCESS(KphOpenDeviceBaseDevice(deviceObject, READ_CONTROL, &deviceBaseObject)))
                    {
                        if (NT_SUCCESS(KphOpenDeviceDriver(deviceBaseObject, READ_CONTROL, &driverObject)))
                        {
                            if (NT_SUCCESS(PhGetDriverName(driverObject, &driverName)))
                            {
                                PhSetListViewSubItem(context->ListViewHandle, EtListViewRowCache[OBJECT_GENERAL_INDEX_DEVICEDRVLOW], 1, PhGetString(driverName));
                                PhDereferenceObject(driverName);
                            }

                            if (NT_SUCCESS(PhGetDriverImageFileName(driverObject, &driverName)))
                            {
                                PhSetListViewSubItem(context->ListViewHandle, EtListViewRowCache[OBJECT_GENERAL_INDEX_DEVICEDRVLOWPATH], 1, PhGetString(driverName));
                                PhDereferenceObject(driverName);
                            }

                            NtClose(driverObject);
                        }
                        NtClose(deviceBaseObject);
                    }

                    NtClose(deviceObject);
                }
            }

            if (driverName = PhGetPnPDeviceName(context->HandleItem->ObjectName))
            {
                ULONG_PTR column_pos = PhFindLastCharInString(driverName, 0, L':');
                PPH_STRING devicePdoName = PH_AUTO(PhSubstring(driverName, 0, column_pos - 5));
                PhSetListViewSubItem(context->ListViewHandle, EtListViewRowCache[OBJECT_GENERAL_INDEX_DEVICEPNPNAME], 1, PhGetString(devicePdoName));
                PhDereferenceObject(driverName);
            }
        }
        else if (PhEqualString2(context->HandleItem->TypeName, L"WindowStation", TRUE))
        {
            HWINSTA hWinStation;
            USEROBJECTFLAGS userFlags;
            PPH_STRING StationType;
            PH_STRINGREF StationName;
            PH_STRINGREF pathPart;

            PhAddListViewGroup(context->ListViewHandle, OBJECT_GENERAL_CATEGORY_WINDOWSTATION, L"Window Station information");

            EtListViewRowCache[OBJECT_GENERAL_INDEX_WINSTATYPE] = PhAddListViewGroupItem(context->ListViewHandle,
                OBJECT_GENERAL_CATEGORY_WINDOWSTATION, OBJECT_GENERAL_INDEX_WINSTATYPE, L"Type", NULL);
            EtListViewRowCache[OBJECT_GENERAL_INDEX_WINSTAVISIBLE] = PhAddListViewGroupItem(context->ListViewHandle,
                OBJECT_GENERAL_CATEGORY_WINDOWSTATION, OBJECT_GENERAL_INDEX_WINSTAVISIBLE, L"Visible", NULL);

            if (!PhIsNullOrEmptyString(context->HandleItem->ObjectName))
            {
                if (!PhSplitStringRefAtLastChar(&context->HandleItem->ObjectName->sr, L'\\', &pathPart, &StationName))
                    StationName = context->HandleItem->ObjectName->sr;
                StationType = PH_AUTO(EtGetWindowStationType(&StationName));
                PhSetListViewSubItem(context->ListViewHandle, EtListViewRowCache[OBJECT_GENERAL_INDEX_WINSTATYPE], 1, PhGetString(StationType));
            }

            if (NT_SUCCESS(EtDuplicateHandleFromProcessEx((PHANDLE)&hWinStation, WINSTA_READATTRIBUTES, context->ProcessId, context->HandleItem->Handle)))
            {
                if (GetUserObjectInformation(
                    hWinStation,
                    UOI_FLAGS,
                    &userFlags,
                    sizeof(USEROBJECTFLAGS),
                    NULL
                    ))
                {
                    PhSetListViewSubItem(context->ListViewHandle, EtListViewRowCache[OBJECT_GENERAL_INDEX_WINSTAVISIBLE], 1,
                        userFlags.dwFlags & WSF_VISIBLE ? L"True" : L"False");
                }
                CloseWindowStation(hWinStation);
            }
        }
        else if (PhEqualString2(context->HandleItem->TypeName, L"Desktop", TRUE))
        {
            HDESK hDesktop;

            PhAddListViewGroup(context->ListViewHandle, OBJECT_GENERAL_CATEGORY_DESKTOP, L"Desktop information");

            EtListViewRowCache[OBJECT_GENERAL_INDEX_DESKTOPIO] = PhAddListViewGroupItem(context->ListViewHandle,
                OBJECT_GENERAL_CATEGORY_DESKTOP, OBJECT_GENERAL_INDEX_DESKTOPIO, L"Input desktop", NULL);
            EtListViewRowCache[OBJECT_GENERAL_INDEX_DESKTOPSID] = PhAddListViewGroupItem(context->ListViewHandle,
                OBJECT_GENERAL_CATEGORY_DESKTOP, OBJECT_GENERAL_INDEX_DESKTOPSID, L"User SID", NULL);
            EtListViewRowCache[OBJECT_GENERAL_INDEX_DESKTOPHEAP] = PhAddListViewGroupItem(context->ListViewHandle,
                OBJECT_GENERAL_CATEGORY_DESKTOP, OBJECT_GENERAL_INDEX_DESKTOPHEAP, L"Heap size", NULL);

            if (NT_SUCCESS(EtDuplicateHandleFromProcessEx((PHANDLE)&hDesktop, DESKTOP_READOBJECTS, context->ProcessId, context->HandleItem->Handle)))
            {
                DWORD nLengthNeeded = 0;
                ULONG vInfo = 0;
                PSID UserSid = NULL;

                if (GetUserObjectInformation(hDesktop, UOI_IO, &vInfo, sizeof(vInfo), NULL))
                {
                    PhSetListViewSubItem(context->ListViewHandle, EtListViewRowCache[OBJECT_GENERAL_INDEX_DESKTOPIO], 1, !!vInfo ? L"True" : L"False");
                }

                GetUserObjectInformation(hDesktop, UOI_USER_SID, NULL, 0, &nLengthNeeded);
                if (nLengthNeeded)
                    UserSid = PhAllocate(nLengthNeeded);
                if (UserSid && GetUserObjectInformation(hDesktop, UOI_USER_SID, UserSid, nLengthNeeded, &nLengthNeeded))
                {
                    PPH_STRING sid = PH_AUTO(PhSidToStringSid(UserSid));
                    PhSetListViewSubItem(context->ListViewHandle, EtListViewRowCache[OBJECT_GENERAL_INDEX_DESKTOPSID], 1, PhGetString(sid));
                    PhFree(UserSid);
                }

                if (GetUserObjectInformation(hDesktop, UOI_HEAPSIZE, &vInfo, sizeof(vInfo), NULL))
                {
                    PPH_STRING size = PH_AUTO(PhFormatSize(vInfo * 1024, ULONG_MAX));
                    PhSetListViewSubItem(context->ListViewHandle, EtListViewRowCache[OBJECT_GENERAL_INDEX_DESKTOPHEAP], 1, PhGetString(size));
                }

                CloseDesktop(hDesktop);
            }
        }
        else if (PhEqualString2(context->HandleItem->TypeName, L"Type", TRUE))
        {
            PH_STRINGREF firstPart;
            PH_STRINGREF typeName;
            POBJECT_TYPES_INFORMATION objectTypes;
            POBJECT_TYPE_INFORMATION objectType;
            ULONG typeIndex;
            PPH_STRING accessString;
            PH_STRING_BUILDER stringBuilder;

            if (PhSplitStringRefAtLastChar(&context->HandleItem->ObjectName->sr, L'\\', &firstPart, &typeName))
            {
                typeIndex = PhGetObjectTypeNumber(&typeName);

                if (typeIndex != ULONG_MAX &&
                    NT_SUCCESS(PhEnumObjectTypes(&objectTypes)))
                {
                    objectType = PH_FIRST_OBJECT_TYPE(objectTypes);

                    for (ULONG i = 0; i < objectTypes->NumberOfTypes; i++)
                    {
                        if (objectType->TypeIndex == typeIndex)
                        {
                            ListView_RemoveGroup(context->ListViewHandle, PH_PLUGIN_HANDLE_GENERAL_CATEGORY_QUOTA);

                            PhAddListViewGroup(context->ListViewHandle, OBJECT_GENERAL_CATEGORY_TYPE, L"Type information");

                            EtListViewRowCache[OBJECT_GENERAL_INDEX_TYPEINDEX] = PhAddListViewGroupItem(context->ListViewHandle,
                                OBJECT_GENERAL_CATEGORY_TYPE, OBJECT_GENERAL_INDEX_TYPEINDEX, L"Index", NULL);
                            EtListViewRowCache[OBJECT_GENERAL_INDEX_TYPEOBJECTS] = PhAddListViewGroupItem(context->ListViewHandle,
                                OBJECT_GENERAL_CATEGORY_TYPE, OBJECT_GENERAL_INDEX_TYPEOBJECTS, L"Objects", NULL);
                            EtListViewRowCache[OBJECT_GENERAL_INDEX_TYPEHANDLES] = PhAddListViewGroupItem(context->ListViewHandle,
                                OBJECT_GENERAL_CATEGORY_TYPE, OBJECT_GENERAL_INDEX_TYPEHANDLES, L"Handles", NULL);
                            EtListViewRowCache[OBJECT_GENERAL_INDEX_TYPEPEAKOBJECTS] = PhAddListViewGroupItem(context->ListViewHandle,
                                OBJECT_GENERAL_CATEGORY_TYPE, OBJECT_GENERAL_INDEX_TYPEPEAKOBJECTS, L"Peak Objects", NULL);
                            EtListViewRowCache[OBJECT_GENERAL_INDEX_TYPEPEAKHANDLES] = PhAddListViewGroupItem(context->ListViewHandle,
                                OBJECT_GENERAL_CATEGORY_TYPE, OBJECT_GENERAL_INDEX_TYPEPEAKHANDLES, L"Peak Handles", NULL);
                            EtListViewRowCache[OBJECT_GENERAL_INDEX_TYPEPOOLTYPE] = PhAddListViewGroupItem(context->ListViewHandle,
                                OBJECT_GENERAL_CATEGORY_TYPE, OBJECT_GENERAL_INDEX_TYPEPOOLTYPE, L"Pool Type", NULL);
                            EtListViewRowCache[OBJECT_GENERAL_INDEX_TYPEPAGECHARGE] = PhAddListViewGroupItem(context->ListViewHandle,
                                OBJECT_GENERAL_CATEGORY_TYPE, OBJECT_GENERAL_INDEX_TYPEPAGECHARGE, L"Default Paged Charge", NULL);
                            EtListViewRowCache[OBJECT_GENERAL_INDEX_TYPENPAGECHARGE] = PhAddListViewGroupItem(context->ListViewHandle,
                                OBJECT_GENERAL_CATEGORY_TYPE, OBJECT_GENERAL_INDEX_TYPENPAGECHARGE, L"Default NP Charge", NULL);

                            PhPrintUInt32(string, objectType->TypeIndex);
                            PhSetListViewSubItem(context->ListViewHandle, EtListViewRowCache[OBJECT_GENERAL_INDEX_TYPEINDEX], 1, string);
                            PhPrintUInt32(string, objectType->TotalNumberOfObjects);
                            PhSetListViewSubItem(context->ListViewHandle, EtListViewRowCache[OBJECT_GENERAL_INDEX_TYPEOBJECTS], 1, string);
                            PhPrintUInt32(string, objectType->TotalNumberOfHandles);
                            PhSetListViewSubItem(context->ListViewHandle, EtListViewRowCache[OBJECT_GENERAL_INDEX_TYPEHANDLES], 1, string);
                            PhPrintUInt32(string, objectType->HighWaterNumberOfObjects);
                            PhSetListViewSubItem(context->ListViewHandle, EtListViewRowCache[OBJECT_GENERAL_INDEX_TYPEPEAKOBJECTS], 1, string);
                            PhPrintUInt32(string, objectType->HighWaterNumberOfHandles);
                            PhSetListViewSubItem(context->ListViewHandle, EtListViewRowCache[OBJECT_GENERAL_INDEX_TYPEPEAKHANDLES], 1, string);
                            PhPrintUInt32(string, objectType->DefaultPagedPoolCharge);

                            PWSTR PoolType = NULL;
                            switch (objectType->PoolType) {
                                case NonPagedPool:
                                    PoolType = L"Non Paged";
                                    break;
                                case PagedPool:
                                    PoolType = L"Paged";
                                    break;
                                case NonPagedPoolNx:
                                    PoolType = L"Non Paged NX";
                                    break;
                                case PagedPoolSessionNx:
                                    PoolType = L"Paged Session NX";
                                    break;
                            }
                            PhSetListViewSubItem(context->ListViewHandle, EtListViewRowCache[OBJECT_GENERAL_INDEX_TYPEPOOLTYPE], 1, PoolType);

                            PhSetListViewSubItem(context->ListViewHandle, EtListViewRowCache[OBJECT_GENERAL_INDEX_TYPEPAGECHARGE], 1, string);
                            PhPrintUInt32(string, objectType->DefaultNonPagedPoolCharge);
                            PhSetListViewSubItem(context->ListViewHandle, EtListViewRowCache[OBJECT_GENERAL_INDEX_TYPENPAGECHARGE], 1, string);

                            PhAddListViewGroup(context->ListViewHandle, OBJECT_GENERAL_CATEGORY_TYPE_ACCESS, L"Type access information");

                            EtListViewRowCache[OBJECT_GENERAL_INDEX_TYPEVALIDMASK] = PhAddListViewGroupItem(context->ListViewHandle,
                                OBJECT_GENERAL_CATEGORY_TYPE_ACCESS, OBJECT_GENERAL_INDEX_TYPEVALIDMASK, L"Valid Access Mask", NULL);
                            EtListViewRowCache[OBJECT_GENERAL_INDEX_TYPEGENERICREAD] = PhAddListViewGroupItem(context->ListViewHandle,
                                OBJECT_GENERAL_CATEGORY_TYPE_ACCESS, OBJECT_GENERAL_INDEX_TYPEGENERICREAD, L"Generic Read", NULL);
                            EtListViewRowCache[OBJECT_GENERAL_INDEX_TYPEGENERICWRITE] = PhAddListViewGroupItem(context->ListViewHandle,
                                OBJECT_GENERAL_CATEGORY_TYPE_ACCESS, OBJECT_GENERAL_INDEX_TYPEGENERICWRITE, L"Generic Write", NULL);
                            EtListViewRowCache[OBJECT_GENERAL_INDEX_TYPEGENERICEXECUTE] = PhAddListViewGroupItem(context->ListViewHandle,
                                OBJECT_GENERAL_CATEGORY_TYPE_ACCESS, OBJECT_GENERAL_INDEX_TYPEGENERICEXECUTE, L"Generic Execute", NULL);
                            EtListViewRowCache[OBJECT_GENERAL_INDEX_TYPEGENERICALL] = PhAddListViewGroupItem(context->ListViewHandle,
                                OBJECT_GENERAL_CATEGORY_TYPE_ACCESS, OBJECT_GENERAL_INDEX_TYPEGENERICALL, L"Generic All", NULL);
                            EtListViewRowCache[OBJECT_GENERAL_INDEX_TYPEINVALIDATTRIBUTES] = PhAddListViewGroupItem(context->ListViewHandle,
                                OBJECT_GENERAL_CATEGORY_TYPE_ACCESS, OBJECT_GENERAL_INDEX_TYPEINVALIDATTRIBUTES, L"Invalid Attributes", NULL);

                            accessString = PH_AUTO(EtGetAccessString2(&typeName, objectType->ValidAccessMask));
                            PhSetListViewSubItem(context->ListViewHandle, EtListViewRowCache[OBJECT_GENERAL_INDEX_TYPEVALIDMASK], 1, PhGetString(accessString));
                            accessString = PH_AUTO(EtGetAccessString2(&typeName, objectType->GenericMapping.GenericRead));
                            PhSetListViewSubItem(context->ListViewHandle, EtListViewRowCache[OBJECT_GENERAL_INDEX_TYPEGENERICREAD], 1, PhGetString(accessString));
                            accessString = PH_AUTO(EtGetAccessString2(&typeName, objectType->GenericMapping.GenericWrite));
                            PhSetListViewSubItem(context->ListViewHandle, EtListViewRowCache[OBJECT_GENERAL_INDEX_TYPEGENERICWRITE], 1, PhGetString(accessString));
                            accessString = PH_AUTO(EtGetAccessString2(&typeName, objectType->GenericMapping.GenericExecute));
                            PhSetListViewSubItem(context->ListViewHandle, EtListViewRowCache[OBJECT_GENERAL_INDEX_TYPEGENERICEXECUTE], 1, PhGetString(accessString));
                            accessString = PH_AUTO(EtGetAccessString2(&typeName, objectType->GenericMapping.GenericAll));
                            PhSetListViewSubItem(context->ListViewHandle, EtListViewRowCache[OBJECT_GENERAL_INDEX_TYPEGENERICALL], 1, PhGetString(accessString));

                            PhInitializeStringBuilder(&stringBuilder, 10);
                            if (objectType->InvalidAttributes & OBJ_KERNEL_HANDLE)
                                PhAppendStringBuilder2(&stringBuilder, L"KERNEL_HANDLE, ");
                            if (objectType->InvalidAttributes & OBJ_OPENLINK)
                                PhAppendStringBuilder2(&stringBuilder, L"OPEN_LINK, ");
                            if (objectType->InvalidAttributes & OBJ_OPENIF)
                                PhAppendStringBuilder2(&stringBuilder, L"OBJ_OPENIF, ");
                            if (objectType->InvalidAttributes & OBJ_PERMANENT)
                                PhAppendStringBuilder2(&stringBuilder, L"OBJ_PERMANENT, ");
                            if (objectType->InvalidAttributes & OBJ_EXCLUSIVE)
                                PhAppendStringBuilder2(&stringBuilder, L"OBJ_EXCLUSIVE, ");
                            if (objectType->InvalidAttributes & OBJ_INHERIT)
                                PhAppendStringBuilder2(&stringBuilder, L"OBJ_INHERIT, ");
                            if (objectType->InvalidAttributes & OBJ_CASE_INSENSITIVE)
                                PhAppendStringBuilder2(&stringBuilder, L"OBJ_CASE_INSENSITIVE, ");
                            if (objectType->InvalidAttributes & OBJ_FORCE_ACCESS_CHECK)
                                PhAppendStringBuilder2(&stringBuilder, L"OBJ_FORCE_ACCESS_CHECK, ");
                            if (objectType->InvalidAttributes & OBJ_IGNORE_IMPERSONATED_DEVICEMAP)
                                PhAppendStringBuilder2(&stringBuilder, L"OBJ_IGNORE_IMPERSONATED_DEVICEMAP, ");
                            if (objectType->InvalidAttributes & OBJ_DONT_REPARSE)
                                PhAppendStringBuilder2(&stringBuilder, L"OBJ_DONT_REPARSE, ");

                            // Remove the trailing ", ".
                            if (PhEndsWithString2(stringBuilder.String, L", ", FALSE))
                                PhRemoveEndStringBuilder(&stringBuilder, 2);

                            accessString = PH_AUTO(PhFinalStringBuilderString(&stringBuilder));
                            PhSetListViewSubItem(context->ListViewHandle, EtListViewRowCache[OBJECT_GENERAL_INDEX_TYPEINVALIDATTRIBUTES], 1, PhGetString(accessString));

                            break;
                        }
                        objectType = PH_NEXT_OBJECT_TYPE(objectType);
                    }
                    PhFree(objectTypes);
                }
            }
        }
    }
}

#define T_WINSTA_INTERACTIVE L"WinSta0"
#define T_WINSTA_SYSTEM L"-0x0-3e7$"
#define T_WINSTA_ANONYMOUS L"-0x0-3e6$"
#define T_WINSTA_LOCALSERVICE L"-0x0-3e5$"
#define T_WINSTA_NETWORK_SERVICE L"-0x0-3e4$"

PPH_STRING EtGetWindowStationType(
    _In_ PPH_STRINGREF StationName
    )
{
    if (PhEqualStringRef2(StationName, T_WINSTA_INTERACTIVE, TRUE))
        return PhCreateString(L"Interactive Window Station");

    PH_FORMAT format[3];

    PhInitFormatC(&format[0], UNICODE_NULL);
    PhInitFormatC(&format[1], L' ');
    PhInitFormatS(&format[2], L"logon session");

    if (PhFindStringInStringRefZ(StationName, T_WINSTA_SYSTEM, TRUE) != SIZE_MAX)
        PhInitFormatS(&format[0], L"System");
    if (PhFindStringInStringRefZ(StationName, T_WINSTA_ANONYMOUS, TRUE) != SIZE_MAX)
        PhInitFormatS(&format[0], L"Anonymous");
    if (PhFindStringInStringRefZ(StationName, T_WINSTA_LOCALSERVICE, TRUE) != SIZE_MAX)
        PhInitFormatS(&format[0], L"Local Service");
    if (PhFindStringInStringRefZ(StationName, T_WINSTA_NETWORK_SERVICE, TRUE) != SIZE_MAX)
        PhInitFormatS(&format[0], L"Network Service");

    return format[0].u.Char != UNICODE_NULL ? PhFormat(format, RTL_NUMBER_OF(format), 0) : NULL;
}

VOID EtHandlePropertiesWindowUninitializing(
    _In_ PVOID Parameter
    )
{
    PPH_PLUGIN_HANDLE_PROPERTIES_WINDOW_CONTEXT context = Parameter;

    if (context->OwnerPlugin == PluginInstance)
    {
        if (context->HandleItem->Handle && context->ProcessId == NtCurrentProcessId())
        {
            PhRemoveItemList(EtObjectManagerOwnHandles, PhFindItemList(EtObjectManagerOwnHandles, context->HandleItem->Handle));
            PhDereferenceObject(EtObjectManagerOwnHandles);

            PhRemoveItemSimpleHashtable(EtObjectManagerPropWindows, context->ParentWindow);
            PhDereferenceObject(EtObjectManagerPropWindows);

            NtClose(context->HandleItem->Handle);
        }

        PhSaveWindowPlacementToSetting(SETTING_NAME_OBJMGR_PROPERTIES_WINDOW_POSITION, NULL, context->ParentWindow);

        PhDereferenceObject(context->HandleItem);
    }
}

static HPROPSHEETPAGE EtpCommonCreatePage(
    _In_ PPH_PLUGIN_HANDLE_PROPERTIES_CONTEXT Context,
    _In_ PWSTR Template,
    _In_ DLGPROC DlgProc
    )
{
    HPROPSHEETPAGE propSheetPageHandle;
    PROPSHEETPAGE propSheetPage;
    PCOMMON_PAGE_CONTEXT pageContext;

    pageContext = PhCreateAlloc(sizeof(COMMON_PAGE_CONTEXT));
    memset(pageContext, 0, sizeof(COMMON_PAGE_CONTEXT));
    pageContext->HandleItem = Context->HandleItem;
    pageContext->ProcessId = Context->ProcessId;

    memset(&propSheetPage, 0, sizeof(PROPSHEETPAGE));
    propSheetPage.dwSize = sizeof(PROPSHEETPAGE);
    propSheetPage.dwFlags = PSP_USECALLBACK;
    propSheetPage.hInstance = PluginInstance->DllBase;
    propSheetPage.pszTemplate = Template;
    propSheetPage.pfnDlgProc = DlgProc;
    propSheetPage.lParam = (LPARAM)pageContext;
    propSheetPage.pfnCallback = EtpCommonPropPageProc;

    propSheetPageHandle = CreatePropertySheetPage(&propSheetPage);
    PhDereferenceObject(pageContext); // already got a ref from above call

    return propSheetPageHandle;
}

INT CALLBACK EtpCommonPropPageProc(
    _In_ HWND hwnd,
    _In_ UINT uMsg,
    _In_ LPPROPSHEETPAGE ppsp
    )
{
    PCOMMON_PAGE_CONTEXT pageContext;

    pageContext = (PCOMMON_PAGE_CONTEXT)ppsp->lParam;

    if (uMsg == PSPCB_ADDREF)
        PhReferenceObject(pageContext);
    else if (uMsg == PSPCB_RELEASE)
        PhDereferenceObject(pageContext);

    return 1;
}

static BOOLEAN NTAPI EnumGenericModulesCallback(
    _In_ PPH_MODULE_INFO Module,
    _In_ PVOID Context
    )
{
    if (
        Module->Type == PH_MODULE_TYPE_MODULE ||
        Module->Type == PH_MODULE_TYPE_WOW64_MODULE
        )
    {
        PhLoadModuleSymbolProvider(
            Context,
            Module->FileName,
            (ULONG64)Module->BaseAddress,
            Module->Size
            );
    }

    return TRUE;
}

INT_PTR CALLBACK EtpTpWorkerFactoryPageDlgProc(
    _In_ HWND hwndDlg,
    _In_ UINT uMsg,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam
    )
{
    switch (uMsg)
    {
    case WM_INITDIALOG:
        {
            LPPROPSHEETPAGE propSheetPage = (LPPROPSHEETPAGE)lParam;
            PCOMMON_PAGE_CONTEXT context = (PCOMMON_PAGE_CONTEXT)propSheetPage->lParam;
            HANDLE workerFactoryHandle;

            if (NT_SUCCESS(EtDuplicateHandleFromProcessEx(&workerFactoryHandle, WORKER_FACTORY_QUERY_INFORMATION, context->ProcessId, context->HandleItem->Handle)))
            {
                WORKER_FACTORY_BASIC_INFORMATION basicInfo;

                if (NT_SUCCESS(NtQueryInformationWorkerFactory(
                    workerFactoryHandle,
                    WorkerFactoryBasicInformation,
                    &basicInfo,
                    sizeof(WORKER_FACTORY_BASIC_INFORMATION),
                    NULL
                    )))
                {
                    PPH_SYMBOL_PROVIDER symbolProvider;
                    PPH_STRING symbol = NULL;
                    WCHAR value[PH_PTR_STR_LEN_1];

                    if (symbolProvider = PhCreateSymbolProvider(NULL))
                    {
                        PhLoadSymbolProviderOptions(symbolProvider);

                        PhEnumGenericModules(
                            basicInfo.ProcessId,
                            NULL,
                            0,
                            EnumGenericModulesCallback,
                            symbolProvider
                            );

                        symbol = PhGetSymbolFromAddress(
                            symbolProvider,
                            (ULONG64)basicInfo.StartRoutine,
                            NULL,
                            NULL,
                            NULL,
                            NULL
                            );

                        PhDereferenceObject(symbolProvider);
                    }

                    if (symbol)
                    {
                        PhSetDialogItemText(
                            hwndDlg,
                            IDC_WORKERTHREADSTART,
                            PhaFormatString(L"Worker Thread Start: %s", symbol->Buffer)->Buffer
                            );
                        PhDereferenceObject(symbol);
                    }
                    else
                    {
                        PhPrintPointer(value, basicInfo.StartRoutine);
                        PhSetDialogItemText(
                            hwndDlg,
                            IDC_WORKERTHREADSTART,
                            PhaFormatString(L"Worker Thread Start: %s", value)->Buffer
                            );
                    }

                    PhPrintPointer(value, basicInfo.StartParameter);
                    PhSetDialogItemText(
                        hwndDlg,
                        IDC_WORKERTHREADCONTEXT,
                        PhaFormatString(L"Worker Thread Context: %s", value)->Buffer
                        );
                }

                NtClose(workerFactoryHandle);
            }

            PhInitializeWindowTheme(hwndDlg, !!PhGetIntegerSetting(L"EnableThemeSupport"));
        }
        break;
    }

    return FALSE;
}

PPH_STRING EtGetAccessString(
    _In_ PPH_STRING TypeName,
    _In_ ACCESS_MASK access
    )
{
    return EtGetAccessStringZ(PhGetStringOrEmpty(TypeName), access);
}

PPH_STRING EtGetAccessString2(
    _In_ PPH_STRINGREF TypeName,
    _In_ ACCESS_MASK access
    )
{
    return EtGetAccessStringZ(PhGetStringRefZ(TypeName), access);
}

PPH_STRING EtGetAccessStringZ(
    _In_ PCWSTR TypeName,
    _In_ ACCESS_MASK access
    )
{
    PPH_STRING AccessString = NULL;
    PPH_ACCESS_ENTRY accessEntries;
    ULONG numberOfAccessEntries;

    if (PhGetAccessEntries(
        TypeName,
        &accessEntries,
        &numberOfAccessEntries
        ))
    {
        PPH_STRING accessString;

        accessString = PH_AUTO(PhGetAccessString(
            access,
            accessEntries,
            numberOfAccessEntries
            ));

        if (accessString->Length != 0)
        {
            AccessString = PhFormatString(
                L"0x%x (%s)",
                access,
                accessString->Buffer
                );
        }
        else
        {
            AccessString = PhFormatString(L"0x%x", access);
        }

        PhFree(accessEntries);
    }
    else
    {
        AccessString = PhFormatString(L"0x%x", access);
    }

    return AccessString;
}

typedef struct _SEARCH_HANDLE_CONTEXT
{
    PPH_STRING SearchObjectName;
    PSYSTEM_HANDLE_TABLE_ENTRY_INFO_EX HandleInfo;
    HANDLE ProcessHandle;
    PPH_LIST SearchResults;
    PPH_QUEUED_LOCK SearchResultsLock;
} SEARCH_HANDLE_CONTEXT, * PSEARCH_HANDLE_CONTEXT;

static NTSTATUS NTAPI EtpSearchHandleFunction(
    _In_ PVOID Parameter
    )
{
    PSEARCH_HANDLE_CONTEXT handleContext = Parameter;
    PPH_STRING ObjectName;

    if (NT_SUCCESS(PhGetHandleInformation(handleContext->ProcessHandle, (HANDLE)handleContext->HandleInfo->HandleValue,
        handleContext->HandleInfo->ObjectTypeIndex, NULL, NULL, &ObjectName, NULL)))
    {
        if (PhStartsWithString(ObjectName, handleContext->SearchObjectName, TRUE))
        {
            PhAcquireQueuedLockExclusive(handleContext->SearchResultsLock);
            PhAddItemList(handleContext->SearchResults, handleContext->HandleInfo);
            PhReleaseQueuedLockExclusive(handleContext->SearchResultsLock);
        }
        PhDereferenceObject(ObjectName);
    }

    PhFree(handleContext);

    return STATUS_SUCCESS;
}

INT EtpEnumObjectHandles(
    _In_ PCOMMON_PAGE_CONTEXT context
    )
{
    static PH_INITONCE initOnce = PH_INITONCE_INIT;
    static ULONG FileTypeIndex;

    if (PhBeginInitOnce(&initOnce))
    {
        FileTypeIndex = PhGetObjectTypeNumberZ(L"File");
        PhEndInitOnce(&initOnce);
    }

    COLORREF ColorNormal = !!PhGetIntegerSetting(L"EnableThemeSupport") ? RGB(43, 43, 43) : GetSysColor(COLOR_WINDOW);
    COLORREF ColorOwnObject = PhGetIntegerSetting(L"ColorOwnProcesses");
    COLORREF ColorInherit = PhGetIntegerSetting(L"ColorInheritHandles");
    COLORREF ColorProtected = PhGetIntegerSetting(L"ColorProtectedHandles");
    COLORREF ColorProtectedInherit = PhGetIntegerSetting(L"ColorPartiallySuspended");

    INT OwnHandlesIndex = 0;
    PSYSTEM_HANDLE_INFORMATION_EX handles;
    ULONG_PTR i;
    ULONG SearchTypeIndex;
    ULONG FindBySameTypeIndex = ULONG_MAX;

    BOOLEAN isDevice = PhEqualString2(context->HandleItem->TypeName, L"Device", TRUE);
    BOOLEAN isAlpcPort = PhEqualString2(context->HandleItem->TypeName, L"ALPC Port", TRUE);
    BOOLEAN isRegKey = PhEqualString2(context->HandleItem->TypeName, L"Key", TRUE);
    BOOLEAN isWinSta = PhEqualString2(context->HandleItem->TypeName, L"WindowStation", TRUE);
    BOOLEAN isTypeObject = PhEqualString2(context->HandleItem->TypeName, L"Type", TRUE);

    SearchTypeIndex = context->HandleItem->TypeIndex;

    if (isTypeObject)
    {
        PH_STRINGREF firstPart;
        PH_STRINGREF typeName;
        ULONG typeIndex;

        if (PhSplitStringRefAtLastChar(&context->HandleItem->ObjectName->sr, L'\\', &firstPart, &typeName))
            if ((typeIndex = PhGetObjectTypeNumber(&typeName)) != ULONG_MAX)
                FindBySameTypeIndex = typeIndex;
    }

    if (NT_SUCCESS(PhEnumHandlesEx(&handles)))
    {
        PPH_LIST SearchResults = PhCreateList(128);
        PH_WORK_QUEUE workQueue;
        PH_QUEUED_LOCK SearchResultsLock = { 0 };

        PPH_HASHTABLE processHandleHashtable = PhCreateSimpleHashtable(8);
        PVOID* processHandlePtr;
        HANDLE processHandle;
        PPH_KEY_VALUE_PAIR procEntry;
        ULONG j = 0;

        PPH_STRING ObjectName;
        BOOLEAN ObjectNameMath;
        BOOLEAN useWorkQueue = isDevice && KsiLevel() < KphLevelMed;

        if (useWorkQueue)
            PhInitializeWorkQueue(&workQueue, 1, 20, 1000);

        for (i = 0; i < handles->NumberOfHandles; i++)
        {
            PSYSTEM_HANDLE_TABLE_ENTRY_INFO_EX handleInfo = &handles->Handles[i];

            // Skip other types
            if (handleInfo->ObjectTypeIndex == SearchTypeIndex ||
                (isDevice && handleInfo->ObjectTypeIndex == FileTypeIndex) ||
                (isTypeObject && handleInfo->ObjectTypeIndex == FindBySameTypeIndex))
            {
                // Skip object name checking if we enumerate handles by type
                if (!(ObjectNameMath = isTypeObject))
                {
                    // Lookup for matches in object name to find more handles for ALPC Port, Device/File, Key, WindowStation
                    if (isAlpcPort || isDevice || isRegKey || isWinSta)
                    {
                        // Open a handle to the process if we don't already have one.
                        processHandlePtr = PhFindItemSimpleHashtable(
                            processHandleHashtable,
                            (PVOID)handleInfo->UniqueProcessId
                            );

                        if (processHandlePtr)
                        {
                            processHandle = (HANDLE)*processHandlePtr;
                        }
                        else
                        {
                            if (NT_SUCCESS(PhOpenProcess(
                                &processHandle,
                                (KsiLevel() >= KphLevelMed ? PROCESS_QUERY_LIMITED_INFORMATION : PROCESS_DUP_HANDLE),
                                (HANDLE)handleInfo->UniqueProcessId
                                )))
                            {
                                PhAddItemSimpleHashtable(
                                    processHandleHashtable,
                                    (PVOID)handleInfo->UniqueProcessId,
                                    processHandle
                                    );
                            }
                            else
                            {
                                continue;
                            }
                        }

                        if (isAlpcPort)
                        {
                            if (NT_SUCCESS(PhGetHandleInformation(processHandle, (HANDLE)handleInfo->HandleValue,
                                handleInfo->ObjectTypeIndex, NULL, NULL, NULL, &ObjectName)))
                            {
                                ObjectNameMath = PhEndsWithString(ObjectName, context->HandleItem->ObjectName, TRUE);   // HACK
                                PhDereferenceObject(ObjectName);
                            }
                        }
                        else
                        {
                            // If we're dealing with a file handle we must take special precautions so we don't hang.
                            if (useWorkQueue)
                            {
                                PSEARCH_HANDLE_CONTEXT searchContext = PhAllocate(sizeof(SEARCH_HANDLE_CONTEXT));
                                searchContext->SearchObjectName = context->HandleItem->ObjectName;
                                searchContext->ProcessHandle = processHandle;
                                searchContext->HandleInfo = handleInfo;
                                searchContext->SearchResults = SearchResults;
                                searchContext->SearchResultsLock = &SearchResultsLock;

                                PhQueueItemWorkQueue(&workQueue, EtpSearchHandleFunction, searchContext);
                                continue;
                            }

                            if (NT_SUCCESS(PhGetHandleInformation(processHandle, (HANDLE)handleInfo->HandleValue,
                                handleInfo->ObjectTypeIndex, NULL, NULL, &ObjectName, NULL)))
                            {
                                ObjectNameMath = PhStartsWithString(ObjectName, context->HandleItem->ObjectName, TRUE);
                                PhDereferenceObject(ObjectName);
                            }
                        }
                    }
                }

                if (handleInfo->Object == context->HandleItem->Object || ObjectNameMath)
                {
                    PhAcquireQueuedLockExclusive(&SearchResultsLock);
                    PhAddItemList(SearchResults, handleInfo);
                    PhReleaseQueuedLockExclusive(&SearchResultsLock);
                }
            }
        }

        if (useWorkQueue)
        {
            PhWaitForWorkQueue(&workQueue);
            PhDeleteWorkQueue(&workQueue);
        }

        while (PhEnumHashtable(processHandleHashtable, &procEntry, &j))
            NtClose((HANDLE)procEntry->Value);

        PhDereferenceObject(processHandleHashtable);

        INT lvItemIndex;
        WCHAR value[PH_INT64_STR_LEN_1];
        PHANDLE_ENTRY entry;
        PPH_STRING columnString;
        CLIENT_ID ClientId = { 0 };

        for (i = 0; i < SearchResults->Count; i++)
        {
            PSYSTEM_HANDLE_TABLE_ENTRY_INFO_EX handleInfo = SearchResults->Items[i];

            // Skip Object Manager own handles
            if ((HANDLE)handleInfo->UniqueProcessId == NtCurrentProcessId() &&
                PhFindItemList(EtObjectManagerOwnHandles, (PVOID)handleInfo->HandleValue) != ULONG_MAX)
            {
                continue;
            }

            entry = PhAllocateZero(sizeof(HANDLE_ENTRY));
            entry->ProcessId = (HANDLE)handleInfo->UniqueProcessId;
            entry->HandleItem = PhCreateHandleItem(handleInfo);
            entry->Color = ColorNormal;
            entry->OwnHandle = handleInfo->Object == context->HandleItem->Object;

            ClientId.UniqueProcess = entry->ProcessId;
            columnString = PH_AUTO(PhGetClientIdName(&ClientId));
            lvItemIndex = PhAddListViewItem(
                context->ListViewHandle,
                entry->OwnHandle ? OwnHandlesIndex++ : MAXINT,     // object own handles first
                PhGetString(columnString), entry
                );

            PhPrintPointer(value, (PVOID)handleInfo->HandleValue);
            PhSetListViewSubItem(context->ListViewHandle, lvItemIndex, ETHNLVC_HANDLE, value);

            columnString = PH_AUTO(EtGetAccessString(context->HandleItem->TypeName, handleInfo->GrantedAccess));
            PhSetListViewSubItem(context->ListViewHandle, lvItemIndex, ETHNLVC_ACCESS, PhGetString(columnString));

            switch (handleInfo->HandleAttributes & (OBJ_PROTECT_CLOSE | OBJ_INHERIT))
            {
                case OBJ_PROTECT_CLOSE:
                    PhSetListViewSubItem(context->ListViewHandle, lvItemIndex, ETHNLVC_ATTRIBUTES, L"Protected");
                    entry->Color = ColorProtected;
                    break;
                case OBJ_INHERIT:
                    PhSetListViewSubItem(context->ListViewHandle, lvItemIndex, ETHNLVC_ATTRIBUTES, L"Inherit");
                    entry->Color = ColorInherit;
                    break;
                case OBJ_PROTECT_CLOSE | OBJ_INHERIT:
                    PhSetListViewSubItem(context->ListViewHandle, lvItemIndex, ETHNLVC_ATTRIBUTES, L"Protected, Inherit");
                    entry->Color = ColorProtectedInherit;
                    break;
            }

            // Highlight own object handles
            if (entry->OwnHandle)
                entry->Color = ColorOwnObject;
        }

        PhDereferenceObject(SearchResults);
        PhFree(handles);
    }

    return OwnHandlesIndex;
}

VOID EtpShowHandleProperties(
    _In_ HWND hwndDlg,
    _In_ PHANDLE_ENTRY entry
    )
{
    HANDLE processHandle;

    if (NT_SUCCESS(PhOpenProcess(
        &processHandle,
        (KsiLevel() >= KphLevelMed ? PROCESS_QUERY_LIMITED_INFORMATION : PROCESS_DUP_HANDLE),
        entry->ProcessId
        )))
    {
        PhGetHandleInformation(
            processHandle,
            entry->HandleItem->Handle,
            entry->HandleItem->TypeIndex,
            NULL,
            &entry->HandleItem->TypeName,
            &entry->HandleItem->ObjectName,
            &entry->HandleItem->BestObjectName
            );

        NtClose(processHandle);
    }

    if (!entry->HandleItem->TypeName)
        entry->HandleItem->TypeName = PhGetObjectTypeIndexName(entry->HandleItem->TypeIndex);

    PhShowHandlePropertiesEx(hwndDlg, entry->ProcessId, entry->HandleItem, (PPH_PLUGIN)((ULONG_PTR)PluginInstance | OBJECT_CHILD_HANDLEPROP_WINDOW), NULL);
}

VOID EtpCloseObjectHandles(
    PCOMMON_PAGE_CONTEXT context,
    PHANDLE_ENTRY* listviewItems,
    ULONG numberOfItems
    )
{
    HANDLE oldHandle;
    NTSTATUS status;

    for (ULONG i = 0; i < numberOfItems; i++)
    {
        if (PhUiCloseHandles(context->WindowHandle, listviewItems[i]->ProcessId, &listviewItems[i]->HandleItem, 1, TRUE))
        {
            if ((status = EtDuplicateHandleFromProcessEx(
                &oldHandle,
                0,
                listviewItems[i]->ProcessId,
                listviewItems[i]->HandleItem->Handle)) == STATUS_INVALID_HANDLE)
            {
                PhRemoveListViewItem(context->ListViewHandle, PhFindListViewItemByParam(context->ListViewHandle, INT_ERROR, listviewItems[i]));
                PhClearReference(&listviewItems[i]->HandleItem);
                PhFree(listviewItems[i]);
            }
            else if (NT_SUCCESS(status))
            {
                NtClose(oldHandle);
            }  
        }
        else
        {
            break;
        }  
    }
}

static COLORREF NTAPI EtpColorItemColorFunction(
    _In_ INT Index,
    _In_ PVOID Param,
    _In_opt_ PVOID Context
    )
{
    PHANDLE_ENTRY entry = Param;

    return entry->Color;
}


INT_PTR CALLBACK EtpObjHandlesPageDlgProc(
    _In_ HWND hwndDlg,
    _In_ UINT uMsg,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam
    )
{
    PCOMMON_PAGE_CONTEXT context = NULL;

    if (uMsg == WM_INITDIALOG)
    {
        LPPROPSHEETPAGE propSheetPage = (LPPROPSHEETPAGE)lParam;
        context = (PCOMMON_PAGE_CONTEXT)propSheetPage->lParam;

        PhSetWindowContext(hwndDlg, PH_WINDOW_CONTEXT_DEFAULT, context);
    }
    else
    {
        context = PhGetWindowContext(hwndDlg, PH_WINDOW_CONTEXT_DEFAULT);
    }

    if (!context)
        return TRUE;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        {
            context->WindowHandle = hwndDlg;
            context->ListViewHandle = GetDlgItem(hwndDlg, IDC_LIST);

            PhSetListViewStyle(context->ListViewHandle, TRUE, TRUE);
            PhSetControlTheme(context->ListViewHandle, L"explorer");
            PhAddListViewColumn(context->ListViewHandle, ETHNLVC_PROCESS, ETHNLVC_PROCESS, ETHNLVC_PROCESS, LVCFMT_LEFT, 118, L"Process");
            PhAddListViewColumn(context->ListViewHandle, ETHNLVC_HANDLE, ETHNLVC_HANDLE, ETHNLVC_HANDLE, LVCFMT_LEFT, 50, L"Handle");
            PhAddListViewColumn(context->ListViewHandle, ETHNLVC_ACCESS, ETHNLVC_ACCESS, ETHNLVC_ACCESS, LVCFMT_LEFT, 120, L"Access");
            PhAddListViewColumn(context->ListViewHandle, ETHNLVC_ATTRIBUTES, ETHNLVC_ATTRIBUTES, ETHNLVC_ATTRIBUTES, LVCFMT_LEFT, 60, L"Attributes");
            PhSetExtendedListView(context->ListViewHandle);
            ExtendedListView_SetSort(context->ListViewHandle, 0, NoSortOrder);
            ExtendedListView_SetItemColorFunction(context->ListViewHandle, EtpColorItemColorFunction);

            ExtendedListView_SetRedraw(context->ListViewHandle, FALSE);

            PhSetCursor(PhLoadCursor(NULL, IDC_WAIT));

            INT OwnHandlesCount = EtpEnumObjectHandles(context);
            INT TotalHandlesCount = ListView_GetItemCount(context->ListViewHandle);

            PhSetCursor(PhLoadCursor(NULL, IDC_ARROW));

            PhInitializeWindowTheme(hwndDlg, !!PhGetIntegerSetting(L"EnableThemeSupport"));

            ExtendedListView_SetRedraw(context->ListViewHandle, TRUE);

            WCHAR string[PH_INT64_STR_LEN_1];
            PhPrintUInt32(string, TotalHandlesCount);
            PhSetWindowText(GetDlgItem(hwndDlg, IDC_OBJ_HANDLESTOTAL), string);
            PhPrintUInt32(string, OwnHandlesCount);
            PhSetWindowText(GetDlgItem(hwndDlg, IDC_OBJ_HANDLESBYOBJECT), string);
            PhPrintUInt32(string, TotalHandlesCount - OwnHandlesCount);
            PhSetWindowText(GetDlgItem(hwndDlg, IDC_OBJ_HANDLESBYNAME), string);

            if (PhEqualString2(context->HandleItem->TypeName, L"Type", TRUE))
                PhSetWindowText(GetDlgItem(hwndDlg, IDC_OBJ_HANDLESBYNAME_L), L"By type:");
        }
        break;
    case WM_DESTROY:
        {
            INT index = INT_ERROR;

            while ((index = PhFindListViewItemByFlags(
                context->ListViewHandle,
                index,
                LVNI_ALL
                )) != INT_ERROR)
            {
                PHANDLE_ENTRY entry;
                if (PhGetListViewItemParam(context->ListViewHandle, index, &entry))
                {
                    PhClearReference(&entry->HandleItem);
                    PhFree(entry);
                }
            }
        }
        break;
    case WM_NCDESTROY:
        {
            PhRemoveWindowContext(hwndDlg, PH_WINDOW_CONTEXT_DEFAULT);
            PhFree(context);
        }
        break;
    case WM_KEYDOWN:
        {
            PHANDLE_ENTRY* listviewItems;
            ULONG numberOfItems;

            switch (LOWORD(wParam))
            {
            case VK_RETURN:
                if (GetFocus() == context->ListViewHandle)
                {
                    PhGetSelectedListViewItemParams(context->ListViewHandle, &listviewItems, &numberOfItems);
                    if (numberOfItems == 1)
                    {
                        if (GetKeyState(VK_CONTROL) < 0)
                        {
                            PPH_PROCESS_ITEM processItem;

                            if (processItem = PhReferenceProcessItem(listviewItems[0]->ProcessId))
                            {
                                SystemInformer_ShowProcessProperties(processItem);
                                PhDereferenceObject(processItem);
                                return TRUE;
                            }
                        }
                        else
                        {
                            EtpShowHandleProperties(hwndDlg, listviewItems[0]);
                            return TRUE;
                        }
                    }
                }
                break;
            case VK_DELETE:
                if (GetFocus() == context->ListViewHandle)
                {
                    PhGetSelectedListViewItemParams(context->ListViewHandle, &listviewItems, &numberOfItems);
                    if (numberOfItems != 0)
                    {
                        EtpCloseObjectHandles(context, listviewItems, numberOfItems);
                        return TRUE;
                    }
                }
                break;
            }
        }
        break;
    case WM_NOTIFY:
        {
            PhHandleListViewNotifyBehaviors(lParam, context->ListViewHandle, PH_LIST_VIEW_DEFAULT_1_BEHAVIORS);

            REFLECT_MESSAGE_DLG(hwndDlg, context->ListViewHandle, uMsg, wParam, lParam);

            LPNMHDR header = (LPNMHDR)lParam;

            if (header->code == NM_DBLCLK)
            {
                if (header->hwndFrom != context->ListViewHandle)
                    break;

                LPNMITEMACTIVATE info = (LPNMITEMACTIVATE)header;
                PHANDLE_ENTRY entry;

                if (entry = PhGetSelectedListViewItemParam(context->ListViewHandle))
                {
                    if (GetKeyState(VK_CONTROL) < 0)
                    {
                        PPH_PROCESS_ITEM processItem;

                        if (processItem = PhReferenceProcessItem(entry->ProcessId))
                        {
                            SystemInformer_ShowProcessProperties(processItem);
                            PhDereferenceObject(processItem);
                        }
                    }
                    else
                    {
                        EtpShowHandleProperties(hwndDlg, entry);
                    }
                }  
            }
        }
        break;
    case WM_CONTEXTMENU:
        {
            PHANDLE_ENTRY* listviewItems;
            ULONG numberOfItems;

            PhGetSelectedListViewItemParams(context->ListViewHandle, &listviewItems, &numberOfItems);
            if (numberOfItems != 0)
            {
                POINT point;
                PPH_EMENU menu;
                PPH_EMENU_ITEM item;
                PPH_EMENU_ITEM propMenuItem;
                PPH_EMENU_ITEM protectedMenuItem;
                PPH_EMENU_ITEM inheritMenuItem;
                PPH_EMENU_ITEM gotoMenuItem;
                ULONG attributes = 0;

                point.x = GET_X_LPARAM(lParam);
                point.y = GET_Y_LPARAM(lParam);

                if (point.x == -1 && point.y == -1)
                    PhGetListViewContextMenuPoint(context->ListViewHandle, &point);

                menu = PhCreateEMenu();

                PhInsertEMenuItem(menu, PhCreateEMenuItem(0, IDC_CLOSEHANDLE, L"C&lose\bDel", NULL, NULL), ULONG_MAX);
                PhInsertEMenuItem(menu, protectedMenuItem = PhCreateEMenuItem(0, IDC_HANDLE_PROTECTED, L"&Protected", NULL, NULL), ULONG_MAX);
                PhInsertEMenuItem(menu, inheritMenuItem = PhCreateEMenuItem(0, IDC_HANDLE_INHERIT, L"&Inherit", NULL, NULL), ULONG_MAX);
                PhInsertEMenuItem(menu, PhCreateEMenuSeparator(), ULONG_MAX);
                PhInsertEMenuItem(menu, gotoMenuItem = PhCreateEMenuItem(0, IDC_GOTOPROCESS, L"&Go to process\bCtrl+Enter", NULL, NULL), ULONG_MAX);
                PhInsertEMenuItem(menu, PhCreateEMenuSeparator(), ULONG_MAX);
                PhInsertEMenuItem(menu, propMenuItem = PhCreateEMenuItem(0, IDC_PROPERTIES, L"Prope&rties\bEnter", NULL, NULL), ULONG_MAX);
                PhInsertEMenuItem(menu, PhCreateEMenuSeparator(), ULONG_MAX);
                PhInsertEMenuItem(menu, PhCreateEMenuItem(0, IDC_COPY, L"&Copy\bCtrl+C", NULL, NULL), ULONG_MAX);
                PhInsertCopyListViewEMenuItem(menu, IDC_COPY, context->ListViewHandle);
                PhSetFlagsEMenuItem(menu, IDC_PROPERTIES, PH_EMENU_DEFAULT, PH_EMENU_DEFAULT);
                if (numberOfItems > 1)
                {
                    PhSetDisabledEMenuItem(protectedMenuItem);
                    PhSetDisabledEMenuItem(inheritMenuItem);
                    PhSetDisabledEMenuItem(propMenuItem);
                    PhSetDisabledEMenuItem(gotoMenuItem);
                }
                else if (numberOfItems == 1)
                {
                    // Re-create the attributes.

                    if (listviewItems[0]->HandleItem->Attributes & OBJ_PROTECT_CLOSE)
                    {
                        attributes |= OBJ_PROTECT_CLOSE;
                        PhSetFlagsEMenuItem(menu, IDC_HANDLE_PROTECTED, PH_EMENU_CHECKED, PH_EMENU_CHECKED);
                    }
                    if (listviewItems[0]->HandleItem->Attributes & OBJ_INHERIT)
                    {
                        attributes |= OBJ_INHERIT;
                        PhSetFlagsEMenuItem(menu, IDC_HANDLE_INHERIT, PH_EMENU_CHECKED, PH_EMENU_CHECKED);
                    } 
                }

                item = PhShowEMenu(
                    menu,
                    hwndDlg,
                    PH_EMENU_SHOW_LEFTRIGHT,
                    PH_ALIGN_LEFT | PH_ALIGN_TOP,
                    point.x,
                    point.y
                    );

                if (item && item->Id != ULONG_MAX && !PhHandleCopyListViewEMenuItem(item))
                {
                    switch (item->Id)
                    {
                        case IDC_CLOSEHANDLE:
                            {
                                EtpCloseObjectHandles(context, listviewItems, numberOfItems);
                            }
                            break;
                        case IDC_HANDLE_PROTECTED:
                        case IDC_HANDLE_INHERIT:
                            {
                                // Toggle the appropriate bit.

                                if (item->Id == IDC_HANDLE_PROTECTED)
                                    attributes ^= OBJ_PROTECT_CLOSE;
                                else if (item->Id == IDC_HANDLE_INHERIT)
                                    attributes ^= OBJ_INHERIT;

                                if (PhUiSetAttributesHandle(hwndDlg, listviewItems[0]->ProcessId, listviewItems[0]->HandleItem, attributes))
                                {
                                    if (item->Id == IDC_HANDLE_PROTECTED)
                                        listviewItems[0]->HandleItem->Attributes ^= OBJ_PROTECT_CLOSE;
                                    else if (item->Id == IDC_HANDLE_INHERIT)
                                        listviewItems[0]->HandleItem->Attributes ^= OBJ_INHERIT;

                                    // Update list row

                                    LONG lvItemIndex = PhFindListViewItemByParam(context->ListViewHandle, INT_ERROR, listviewItems[0]);

                                    switch (listviewItems[0]->HandleItem->Attributes & (OBJ_PROTECT_CLOSE | OBJ_INHERIT))
                                    {
                                        case OBJ_PROTECT_CLOSE:
                                            PhSetListViewSubItem(context->ListViewHandle, lvItemIndex, ETHNLVC_ATTRIBUTES, L"Protected");
                                            listviewItems[0]->Color = PhGetIntegerSetting(L"ColorProtectedHandles");
                                            break;
                                        case OBJ_INHERIT:
                                            PhSetListViewSubItem(context->ListViewHandle, lvItemIndex, ETHNLVC_ATTRIBUTES, L"Inherit");
                                            listviewItems[0]->Color = PhGetIntegerSetting(L"ColorInheritHandles");
                                            break;
                                        case OBJ_PROTECT_CLOSE | OBJ_INHERIT:
                                            PhSetListViewSubItem(context->ListViewHandle, lvItemIndex, ETHNLVC_ATTRIBUTES, L"Protected, Inherit");
                                            listviewItems[0]->Color = PhGetIntegerSetting(L"ColorPartiallySuspended");
                                            break;
                                        default:
                                            PhSetListViewSubItem(context->ListViewHandle, lvItemIndex, ETHNLVC_ATTRIBUTES, NULL);
                                            listviewItems[0]->Color = !!PhGetIntegerSetting(L"EnableThemeSupport") ? RGB(43, 43, 43) : GetSysColor(COLOR_WINDOW);
                                            break;
                                    }

                                    if (listviewItems[0]->OwnHandle)
                                        listviewItems[0]->Color = PhGetIntegerSetting(L"ColorOwnProcesses");

                                    ListView_SetItemState(context->ListViewHandle, -1, 0, LVIS_SELECTED);
                                }
                            }
                            break;
                        case IDC_PROPERTIES:
                            {
                                EtpShowHandleProperties(hwndDlg, listviewItems[0]);
                            }
                            break;
                        case IDC_GOTOPROCESS:
                            {
                                PPH_PROCESS_ITEM processItem;

                                if (processItem = PhReferenceProcessItem(listviewItems[0]->ProcessId))
                                {
                                    SystemInformer_ShowProcessProperties(processItem);
                                    PhDereferenceObject(processItem);
                                }
                            }
                            break;
                        case IDC_COPY:
                            {
                                PhCopyListView(context->ListViewHandle);
                            }
                            break;
                    }

                    PhDestroyEMenu(menu);
                }
            }
        }
        break;
    } 

    return FALSE;
}

static BOOL CALLBACK EtpEnumDesktopsCallback(
    _In_ LPWSTR lpszDesktop,
    _In_ LPARAM lParam
    )
{
    PCOMMON_PAGE_CONTEXT context = (PCOMMON_PAGE_CONTEXT)lParam;
    HDESK hDesktop;
    UINT lvItemIndex;

    lvItemIndex = PhAddListViewItem(context->ListViewHandle, MAXINT, lpszDesktop, PhCreateString(lpszDesktop));

    if (hDesktop = OpenDesktop(lpszDesktop, 0, FALSE, DESKTOP_READOBJECTS))
    {
        DWORD nLengthNeeded = 0;
        ULONG vInfo = 0;
        PSID UserSid = NULL;
        
        GetUserObjectInformation(hDesktop, UOI_USER_SID, NULL, 0, &nLengthNeeded);
        if (nLengthNeeded)
            UserSid = PhAllocate(nLengthNeeded);
        if (UserSid && GetUserObjectInformation(hDesktop, UOI_USER_SID, UserSid, nLengthNeeded, &nLengthNeeded))
        {
            PPH_STRING sid = PH_AUTO(PhSidToStringSid(UserSid));
            PhSetListViewSubItem(context->ListViewHandle, lvItemIndex, ETDTLVC_SID, PhGetString(sid));
            PhFree(UserSid);
        }

        if (GetUserObjectInformation(hDesktop, UOI_HEAPSIZE, &vInfo, sizeof(vInfo), NULL))
        {
            PPH_STRING size = PH_AUTO(PhFormatString(L"%d MB", vInfo / 1024));
            PhSetListViewSubItem(context->ListViewHandle, lvItemIndex, ETDTLVC_HEAP, PhGetString(size));
        }

        if (GetUserObjectInformation(hDesktop, UOI_IO, &vInfo, sizeof(vInfo), NULL))
        {
            PhSetListViewSubItem(context->ListViewHandle, lvItemIndex, ETDTLVC_IO, !!vInfo ? L"True" : L"False");
        }

        CloseDesktop(hDesktop);
    }

    return TRUE;
}

typedef struct _OPEN_DESKTOP_CONTEXT
{
    PPH_STRING DesktopName;
    HWINSTA DesktopWinStation;
    HWINSTA CurrentWinStation;
} OPEN_DESKTOP_CONTEXT, * POPEN_DESKTOP_CONTEXT;

static NTSTATUS EtpOpenSecurityDesktopHandle(
    _Inout_ PHANDLE Handle,
    _In_ ACCESS_MASK DesiredAccess,
    _In_opt_ PVOID Context
    )
{
    POPEN_DESKTOP_CONTEXT context = Context;
    NTSTATUS status = STATUS_UNSUCCESSFUL;
    HDESK desktopHandle;

    if (context->DesktopWinStation) SetProcessWindowStation(context->DesktopWinStation);
    if (desktopHandle = OpenDesktop(
        PhGetString(context->DesktopName),
        0,
        FALSE,
        MAXIMUM_ALLOWED
        ))
    {
        *Handle = (HANDLE)desktopHandle;
        status = STATUS_SUCCESS;
    }
    if (context->DesktopWinStation) SetProcessWindowStation(context->CurrentWinStation);

    return status;
}

static NTSTATUS EtpCloseSecurityDesktop(
    _In_ PVOID Context
    )
{
    POPEN_DESKTOP_CONTEXT context = Context;

    PhClearReference(&context->DesktopName);
    if (context->DesktopWinStation) CloseWindowStation(context->DesktopWinStation);
    PhFree(context);
    return STATUS_SUCCESS;
}

VOID EtpOpenDesktopSecurity(
    PCOMMON_PAGE_CONTEXT context,
    PPH_STRING deskName
    )
{
    POPEN_DESKTOP_CONTEXT OpenContext = PhAllocateZero(sizeof(OPEN_DESKTOP_CONTEXT));
    HWINSTA hWinStation;

    OpenContext->DesktopName = PhReferenceObject(deskName);
    OpenContext->CurrentWinStation = GetProcessWindowStation();
    if (NT_SUCCESS(EtDuplicateHandleFromProcessEx((PHANDLE)&hWinStation, WINSTA_ENUMDESKTOPS, context->ProcessId, context->HandleItem->Handle)))
    {
        if (NtCompareObjects((HANDLE)OpenContext->CurrentWinStation, (HANDLE)hWinStation) == STATUS_NOT_SAME_OBJECT)
            OpenContext->DesktopWinStation = hWinStation;
        else
            CloseWindowStation(hWinStation);
    }

    PhEditSecurity(
        !!PhGetIntegerSetting(L"ForceNoParent") ? NULL : context->WindowHandle,
        PhGetString(deskName),
        L"Desktop",
        EtpOpenSecurityDesktopHandle,
        EtpCloseSecurityDesktop,
        OpenContext
        );
}

INT_PTR CALLBACK EtpWinStaPageDlgProc(
    _In_ HWND hwndDlg,
    _In_ UINT uMsg,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam
    )
{
    PCOMMON_PAGE_CONTEXT context = NULL;

    if (uMsg == WM_INITDIALOG)
    {
        LPPROPSHEETPAGE propSheetPage = (LPPROPSHEETPAGE)lParam;
        context = (PCOMMON_PAGE_CONTEXT)propSheetPage->lParam;

        PhSetWindowContext(hwndDlg, PH_WINDOW_CONTEXT_DEFAULT, context);
    }
    else
    {
        context = PhGetWindowContext(hwndDlg, PH_WINDOW_CONTEXT_DEFAULT);
    }

    if (!context)
        return TRUE;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        {
            HWINSTA hWinStation;
            context->WindowHandle = hwndDlg;
            context->ListViewHandle = GetDlgItem(hwndDlg, IDC_LIST);

            PhSetListViewStyle(context->ListViewHandle, TRUE, TRUE);
            PhSetControlTheme(context->ListViewHandle, L"explorer");
            PhAddListViewColumn(context->ListViewHandle, ETDTLVC_NAME, ETDTLVC_NAME, ETDTLVC_NAME, LVCFMT_LEFT, 152, L"Name");
            PhAddListViewColumn(context->ListViewHandle, ETDTLVC_SID, ETDTLVC_SID, ETDTLVC_SID, LVCFMT_LEFT, 105, L"SID");
            PhAddListViewColumn(context->ListViewHandle, ETDTLVC_HEAP, ETDTLVC_HEAP, ETDTLVC_HEAP, LVCFMT_LEFT, 62, L"Heap Size");
            PhAddListViewColumn(context->ListViewHandle, ETDTLVC_IO, ETDTLVC_IO, ETDTLVC_IO, LVCFMT_LEFT, 45, L"Input");
            PhSetExtendedListView(context->ListViewHandle);
            ExtendedListView_SetSort(context->ListViewHandle, 0, NoSortOrder);

            ExtendedListView_SetRedraw(context->ListViewHandle, FALSE);

            if (NT_SUCCESS(EtDuplicateHandleFromProcessEx((PHANDLE)&hWinStation, WINSTA_ENUMDESKTOPS, context->ProcessId, context->HandleItem->Handle)))
            {
                HWINSTA currentStation = GetProcessWindowStation();
                NTSTATUS status = NtCompareObjects((HANDLE)currentStation, (HANDLE)hWinStation);
                if (status == STATUS_NOT_SAME_OBJECT)
                    SetProcessWindowStation(hWinStation);
                EnumDesktops(hWinStation, EtpEnumDesktopsCallback, (LPARAM)context);
                if (status == STATUS_NOT_SAME_OBJECT)
                    SetProcessWindowStation(currentStation);
                CloseWindowStation(hWinStation);
            }

            ExtendedListView_SetRedraw(context->ListViewHandle, TRUE);

            PhInitializeWindowTheme(hwndDlg, !!PhGetIntegerSetting(L"EnableThemeSupport"));
        }
        break;
    case WM_DESTROY:
        {
            INT index = INT_ERROR;

            while ((index = PhFindListViewItemByFlags(
                context->ListViewHandle,
                index,
                LVNI_ALL
                )) != INT_ERROR)
            {
                PPH_STRING deskName;
                if (PhGetListViewItemParam(context->ListViewHandle, index, &deskName))
                {
                    PhDereferenceObject(deskName);
                }
            }
        }
        break;
    case WM_NCDESTROY:
        {
            PhRemoveWindowContext(hwndDlg, PH_WINDOW_CONTEXT_DEFAULT);
            PhFree(context);
        }
        break;
    case WM_NOTIFY:
        {
            PhHandleListViewNotifyBehaviors(lParam, context->ListViewHandle, PH_LIST_VIEW_DEFAULT_1_BEHAVIORS);
            REFLECT_MESSAGE_DLG(hwndDlg, context->ListViewHandle, uMsg, wParam, lParam);

            LPNMHDR header = (LPNMHDR)lParam;

            if (header->code == NM_DBLCLK)
            {
                if (header->hwndFrom != context->ListViewHandle)
                    break;

                LPNMITEMACTIVATE info = (LPNMITEMACTIVATE)header;
                PPH_STRING deskName;

                if (deskName = PhGetSelectedListViewItemParam(context->ListViewHandle))
                {
                    EtpOpenDesktopSecurity(context, deskName);
                }
            }
        }
        break;
    case WM_CONTEXTMENU:
        {
            PVOID* listviewItems;
            ULONG numberOfItems;

            PhGetSelectedListViewItemParams(context->ListViewHandle, &listviewItems, &numberOfItems);
            if (numberOfItems != 0)
            {
                POINT point;
                PPH_EMENU menu;
                PPH_EMENU_ITEM item;
                PPH_EMENU_ITEM secMenuItem;
                HWND ListViewHandle = GetDlgItem(hwndDlg, IDC_LIST);

                point.x = GET_X_LPARAM(lParam);
                point.y = GET_Y_LPARAM(lParam);

                if (point.x == -1 && point.y == -1)
                    PhGetListViewContextMenuPoint(ListViewHandle, &point);

                menu = PhCreateEMenu();

                PhInsertEMenuItem(menu, secMenuItem = PhCreateEMenuItem(0, IDC_SECURITY, L"&Security\bEnter", NULL, NULL), ULONG_MAX);
                PhInsertEMenuItem(menu, PhCreateEMenuSeparator(), ULONG_MAX);
                PhInsertEMenuItem(menu, PhCreateEMenuItem(0, IDC_COPY, L"&Copy\bCtrl+C", NULL, NULL), ULONG_MAX);
                PhInsertCopyListViewEMenuItem(menu, IDC_COPY, ListViewHandle);
                PhSetFlagsEMenuItem(menu, IDC_SECURITY, PH_EMENU_DEFAULT, PH_EMENU_DEFAULT);
                if (numberOfItems > 1)
                    PhSetDisabledEMenuItem(secMenuItem);

                item = PhShowEMenu(
                    menu,
                    hwndDlg,
                    PH_EMENU_SHOW_LEFTRIGHT,
                    PH_ALIGN_LEFT | PH_ALIGN_TOP,
                    point.x,
                    point.y
                    );

                if (item && item->Id != ULONG_MAX && !PhHandleCopyListViewEMenuItem(item))
                {
                    switch (item->Id)
                    {
                    case IDC_COPY:
                        PhCopyListView(ListViewHandle);
                        break;
                    case IDC_SECURITY:
                        EtpOpenDesktopSecurity(context, listviewItems[0]);
                        break;
                    }
                }

                PhDestroyEMenu(menu);
            }
        }
        break;
    case WM_KEYDOWN:
        {
            PPH_STRING* listviewItems;
            ULONG numberOfItems;

            switch (LOWORD(wParam))
            {
            case VK_RETURN:
                if (GetFocus() == context->ListViewHandle)
                {
                    PhGetSelectedListViewItemParams(context->ListViewHandle, &listviewItems, &numberOfItems);
                    if (numberOfItems == 1)
                    {
                        EtpOpenDesktopSecurity(context, listviewItems[0]);
                        return TRUE;
                    }
                }
                break;
            }
            break;
        }
    }

    return FALSE;
}
