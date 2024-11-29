/*
 * Copyright (c) 2022 Winsider Seminars & Solutions, Inc.  All rights reserved.
 *
 * This file is part of System Informer.
 *
 * Authors:
 *
 *     jxy-s
 *
 */

namespace CustomBuildTool
{
    public static class DynData
    {
        private const string FileHeader =
@"/*
 * Copyright (c) 2022 Winsider Seminars & Solutions, Inc.  All rights reserved.
 *
 * This file is part of System Informer.
 *
 * THIS IS AN AUTOGENERATED FILE, DO NOT MODIFY
 *
 */";

        private const string Includes =
@"#include <kphlibbase.h>";

        private const UInt32 Version = 15;

        private static readonly byte[] SessionTokenPublicKey = new byte[]
        {
            0x52, 0x53, 0x41, 0x31, 0x00, 0x10, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
            0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x01, 0x00, 0x01, 0xDE, 0x4C, 0x64, 0xCA, 0x4E, 0xEA, 0x1C, 0x06, 0xBD,
            0x40, 0x42, 0x46, 0x3A, 0x80, 0x2E, 0xB1, 0x13, 0xCE, 0x53, 0x6B, 0x00,
            0xC0, 0x53, 0x79, 0x54, 0xB2, 0xF3, 0xC2, 0xAD, 0x21, 0xFD, 0xB4, 0x06,
            0xB2, 0xA7, 0x8F, 0x75, 0xC4, 0xF2, 0xC9, 0x6B, 0x30, 0x7E, 0x46, 0x48,
            0x70, 0x58, 0x38, 0x26, 0x14, 0x63, 0xC9, 0xB7, 0xB6, 0x28, 0x90, 0xA5,
            0x3E, 0x03, 0xFC, 0x30, 0xD7, 0xE0, 0xF9, 0x06, 0xC1, 0x08, 0x5A, 0x7A,
            0x1D, 0x64, 0x2B, 0x2C, 0x21, 0x25, 0x4D, 0x97, 0x22, 0x66, 0xD6, 0x2B,
            0x16, 0x4B, 0x83, 0xA7, 0x00, 0x40, 0xFF, 0xD4, 0xB3, 0xB9, 0xED, 0x6B,
            0x11, 0x89, 0x87, 0x7B, 0xAA, 0xFD, 0x27, 0x9D, 0x9C, 0x2C, 0x2C, 0x6D,
            0xAD, 0x8E, 0xD7, 0xBC, 0xE2, 0x0A, 0xCE, 0x27, 0x08, 0x35, 0xDE, 0x32,
            0x77, 0xAC, 0xC6, 0x21, 0xBA, 0x06, 0xEB, 0xD2, 0xCE, 0x5B, 0xF2, 0x14,
            0x8F, 0x7C, 0x20, 0x99, 0x43, 0x25, 0xD4, 0x3F, 0xDD, 0x7E, 0xBB, 0xE4,
            0xF8, 0x3B, 0x3B, 0xB9, 0x2F, 0x74, 0xA5, 0xAF, 0xA4, 0x47, 0x8A, 0x21,
            0x83, 0x4D, 0x40, 0x69, 0x55, 0x6F, 0xEF, 0x26, 0x9C, 0x76, 0x80, 0xE0,
            0x4E, 0x12, 0x95, 0xB6, 0x8C, 0xFA, 0x4B, 0x4D, 0x6D, 0x8A, 0x37, 0xB6,
            0xDD, 0x21, 0xD7, 0x99, 0x0F, 0xEB, 0x7B, 0x95, 0xE8, 0xC9, 0x39, 0x58,
            0xBB, 0x56, 0x3B, 0x3C, 0x21, 0x63, 0x29, 0xDB, 0x36, 0xE2, 0x81, 0xF2,
            0x46, 0x61, 0xB6, 0xEC, 0xDB, 0x19, 0x73, 0xE5, 0x25, 0x12, 0xBE, 0x98,
            0x99, 0x0F, 0x52, 0x75, 0xFA, 0x8A, 0x12, 0x52, 0xE1, 0x01, 0x82, 0x29,
            0x79, 0x69, 0x7B, 0x2B, 0xDB, 0xBD, 0x7F, 0x0A, 0x87, 0xAD, 0x9A, 0x1F,
            0xC0, 0xC1, 0x6C, 0x96, 0x7D, 0x84, 0x86, 0xAD, 0x53, 0x64, 0x7D, 0x88,
            0x67, 0xE2, 0x56, 0x7F, 0x89, 0x6C, 0x00, 0x30, 0xC7, 0xEB, 0x93, 0xCF,
            0x58, 0xB0, 0xE3, 0xDC, 0x4D, 0x23, 0xAD, 0x78, 0x8B, 0xAD, 0xFC, 0x1D,
            0x09, 0x49, 0x92, 0xCB, 0x7A, 0x0C, 0x6E, 0xCA, 0x50, 0xAB, 0xDD, 0x0E,
            0xBA, 0xE4, 0x8B, 0x87, 0x16, 0x94, 0x66, 0x03, 0x0D, 0x07, 0xDC, 0xB8,
            0x06, 0xE7, 0x29, 0xC5, 0x57, 0xEE, 0x65, 0xF2, 0x7E, 0x85, 0xC2, 0x49,
            0x28, 0xA3, 0x4E, 0x69, 0x3E, 0xAD, 0x81, 0x2D, 0x47, 0xC8, 0x31, 0xA4,
            0xAE, 0x3B, 0x23, 0xF6, 0x33, 0x5F, 0xA3, 0xA2, 0xBF, 0x77, 0xFE, 0x98,
            0x76, 0xB0, 0x37, 0xD6, 0x08, 0x09, 0x8E, 0x6E, 0x8C, 0xFB, 0x77, 0x87,
            0xF2, 0x29, 0xC2, 0x00, 0xB9, 0xAD, 0x2A, 0x71, 0x5B, 0x64, 0x1F, 0x06,
            0x2B, 0x18, 0x17, 0xB7, 0x90, 0xA4, 0xEF, 0xF2, 0x64, 0x26, 0x20, 0xE4,
            0x15, 0xE0, 0xEC, 0xD6, 0x86, 0xDE, 0x70, 0x8A, 0xCD, 0x57, 0xFE, 0xB2,
            0xD3, 0x16, 0xBF, 0xD4, 0x72, 0x5B, 0x26, 0xD6, 0x80, 0x84, 0x10, 0xAB,
            0xD1, 0x12, 0x4D, 0x84, 0xCA, 0x99, 0x22, 0x72, 0x7D, 0x95, 0xDE, 0x31,
            0x0E, 0x01, 0x80, 0x3F, 0xCE, 0x5D, 0x11, 0xF8, 0xA1, 0x66, 0x2A, 0xDF,
            0x42, 0x24, 0x69, 0x3E, 0x47, 0x8D, 0xB7, 0x96, 0xB2, 0x36, 0x70, 0x41,
            0x5F, 0xC8, 0x63, 0x5F, 0xA1, 0x39, 0x2D, 0x2B, 0xDD, 0xE2, 0xF5, 0xB9,
            0x8D, 0xE0, 0x91, 0x73, 0xA8, 0xCA, 0xBF, 0xA7, 0x77, 0x61, 0x9F, 0xA2,
            0x30, 0x18, 0x31, 0xDB, 0x79, 0x89, 0xED, 0xE1, 0x89, 0x45, 0x52, 0x05,
            0x7C, 0xD7, 0x11, 0x15, 0x45, 0xD4, 0x19, 0xE1, 0xF4, 0x23, 0xD9, 0x9D,
            0xB0, 0x80, 0x19, 0xB8, 0x79, 0x20, 0x8C, 0xCB, 0xA3, 0xE3, 0x53, 0x4E,
            0x8B, 0x2E, 0xC1, 0x9C, 0x0A, 0x53, 0x1A, 0x14, 0x65, 0x71, 0xC4, 0x66,
            0x4A, 0x82, 0x8A, 0xF3, 0x67, 0x50, 0xFA, 0xB7, 0x3A, 0x25, 0x61
        };

        private enum Class
        {
            Ntoskrnl = 0,
            Ntkrla57 = 1,
            Lxcore = 2,
        };

        private static Class ClassFromString(string input)
        {
            switch (input)
            {
                case "ntoskrnl.exe": return Class.Ntoskrnl;
                case "ntkrla57.exe": return Class.Ntkrla57;
                case "lxcore.sys": return Class.Lxcore;
                default: throw new Exception($"invalid file name {input}");
            }
        }

        private static UInt16 MachineFromString(string input)
        {
            switch (input)
            {
                case "amd64": return 0x8664;
                case "arm64": return 0xAA64;
                default: throw new Exception($"invalid machine {input}");
            }
        }

        private static string DynConfigC =
$@"
#define KPH_DYN_CONFIGURATION_VERSION           ((ULONG){Version})
#define KPH_DYN_SESSION_TOKEN_PUBLIC_KEY_LENGTH ((ULONG){SessionTokenPublicKey.Length})
#define KPH_DYN_CLASS_NTOSKRNL                  ((USHORT){(UInt16)Class.Ntoskrnl})
#define KPH_DYN_CLASS_NTKRLA57                  ((USHORT){(UInt16)Class.Ntkrla57})
#define KPH_DYN_CLASS_LXCORE                    ((USHORT){(UInt16)Class.Lxcore})

#include <pshpack1.h>

typedef struct _KPH_DYN_KERNEL_FIELDS
{{
    USHORT EgeGuid;                      // dt nt!_ETW_GUID_ENTRY Guid
    USHORT EpObjectTable;                // dt nt!_EPROCESS ObjectTable
    USHORT EreGuidEntry;                 // dt nt!_ETW_REG_ENTRY GuidEntry
    USHORT HtHandleContentionEvent;      // dt nt!_HANDLE_TABLE HandleContentionEvent
    USHORT OtName;                       // dt nt!_OBJECT_TYPE Name
    USHORT OtIndex;                      // dt nt!_OBJECT_TYPE Index
    USHORT ObDecodeShift;                // dt nt!_HANDLE_TABLE_ENTRY ObjectPointerBits
    USHORT ObAttributesShift;            // dt nt!_HANDLE_TABLE_ENTRY Attributes
    USHORT AlpcCommunicationInfo;        // dt nt!_ALPC_PORT CommunicationInfo
    USHORT AlpcOwnerProcess;             // dt nt!_ALPC_PORT OwnerProcess
    USHORT AlpcConnectionPort;           // dt nt!_ALPC_COMMUNICATION_INFO ConnectionPort
    USHORT AlpcServerCommunicationPort;  // dt nt!_ALPC_COMMUNICATION_INFO ServerCommunicationPort
    USHORT AlpcClientCommunicationPort;  // dt nt!_ALPC_COMMUNICATION_INFO ClientCommunicationPort
    USHORT AlpcHandleTable;              // dt nt!_ALPC_COMMUNICATION_INFO HandleTable
    USHORT AlpcHandleTableLock;          // dt nt!_ALPC_HANDLE_TABLE Lock
    USHORT AlpcAttributes;               // dt nt!_ALPC_PORT PortAttributes
    USHORT AlpcAttributesFlags;          // dt nt!_ALPC_PORT_ATTRIBUTES Flags
    USHORT AlpcPortContext;              // dt nt!_ALPC_PORT PortContext
    USHORT AlpcPortObjectLock;           // dt nt!_ALPC_PORT PortObjectLock
    USHORT AlpcSequenceNo;               // dt nt!_ALPC_PORT SequenceNo
    USHORT AlpcState;                    // dt nt!_ALPC_PORT u1.State
    USHORT KtReadOperationCount;         // dt nt!_KTHREAD ReadOperationCount
    USHORT KtWriteOperationCount;        // dt nt!_KTHREAD WriteOperationCount
    USHORT KtOtherOperationCount;        // dt nt!_KTHREAD OtherOperationCount
    USHORT KtReadTransferCount;          // dt nt!_KTHREAD ReadTransferCount
    USHORT KtWriteTransferCount;         // dt nt!_KTHREAD WriteTransferCount
    USHORT KtOtherTransferCount;         // dt nt!_KTHREAD OtherTransferCount
    USHORT MmSectionControlArea;         // dt nt!_SECTION u1.ControlArea
    USHORT MmControlAreaListHead;        // dt nt!_CONTROL_AREA ListHead
    USHORT MmControlAreaLock;            // dt nt!_CONTROL_AREA ControlAreaLock
    USHORT EpSectionObject;              // dt nt!_EPROCESS SectionObject
}} KPH_DYN_KERNEL_FIELDS, *PKPH_DYN_KERNEL_FIELDS;

typedef KPH_DYN_KERNEL_FIELDS KPH_DYN_NTOSKRNL_FIELDS;
typedef PKPH_DYN_KERNEL_FIELDS PKPH_DYN_NTOSKRNL_FIELDS;
typedef KPH_DYN_KERNEL_FIELDS KPH_DYN_NTKRLA57_FIELDS;
typedef PKPH_DYN_KERNEL_FIELDS PKPH_DYN_NTKRLA57_FIELDS;

typedef struct _KPH_DYN_LXCORE_FIELDS
{{
    USHORT LxPicoProc;                   // uf lxcore!LxpSyscall_GETPID
    USHORT LxPicoProcInfo;               // uf lxcore!LxpSyscall_GETPID
    USHORT LxPicoProcInfoPID;            // uf lxcore!LxpSyscall_GETPID
    USHORT LxPicoThrdInfo;               // uf lxcore!LxpSyscall_GETTID
    USHORT LxPicoThrdInfoTID;            // uf lxcore!LxpSyscall_GETTID
}} KPH_DYN_LXCORE_FIELDS, *PKPH_DYN_LXCORE_FIELDS;

typedef struct _KPH_DYN_FIELDS
{{
    ULONG FieldsId;
    USHORT Length;
    BYTE Fields[ANYSIZE_ARRAY];
}} KPH_DYN_FIELDS, *PKPH_DYN_FIELDS;

typedef struct _KPH_DYN_DATA
{{
    USHORT Class;
    USHORT Machine;
    ULONG TimeDateStamp;
    ULONG SizeOfImage;
    ULONG Offset;
}} KPH_DYN_DATA, *PKPH_DYN_DATA;

typedef struct _KPH_DYN_CONFIG
{{
    ULONG Version;
    BYTE SessionTokenPublicKey[KPH_DYN_SESSION_TOKEN_PUBLIC_KEY_LENGTH];
    ULONG Count;
    KPH_DYN_DATA Data[ANYSIZE_ARRAY];
    // BYTE Fields[ANYSIZE_ARRAY];
}} KPH_DYN_CONFIG, *PKPH_DYN_CONFIG;

#include <poppack.h>";

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct DynDataEntry
        {
            public UInt16 Class;
            public UInt16 Machine;
            public UInt32 TimeDateStamp;
            public UInt32 SizeOfImage;
            public UInt32 Offset;
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct DynFieldsKernel
        {
            public UInt16 EgeGuid;
            public UInt16 EpObjectTable;
            public UInt16 EreGuidEntry;
            public UInt16 HtHandleContentionEvent;
            public UInt16 OtName;
            public UInt16 OtIndex;
            public UInt16 ObDecodeShift;
            public UInt16 ObAttributesShift;
            public UInt16 AlpcCommunicationInfo;
            public UInt16 AlpcOwnerProcess;
            public UInt16 AlpcConnectionPort;
            public UInt16 AlpcServerCommunicationPort;
            public UInt16 AlpcClientCommunicationPort;
            public UInt16 AlpcHandleTable;
            public UInt16 AlpcHandleTableLock;
            public UInt16 AlpcAttributes;
            public UInt16 AlpcAttributesFlags;
            public UInt16 AlpcPortContext;
            public UInt16 AlpcPortObjectLock;
            public UInt16 AlpcSequenceNo = UInt16.MaxValue;
            public UInt16 AlpcState;
            public UInt16 KtReadOperationCount;
            public UInt16 KtWriteOperationCount;
            public UInt16 KtOtherOperationCount;
            public UInt16 KtReadTransferCount;
            public UInt16 KtWriteTransferCount;
            public UInt16 KtOtherTransferCount;
            public UInt16 MmSectionControlArea;
            public UInt16 MmControlAreaListHead;
            public UInt16 MmControlAreaLock;
            public UInt16 EpSectionObject;

            public DynFieldsKernel()
            {
                EgeGuid = UInt16.MaxValue;
                EpObjectTable = UInt16.MaxValue;
                EreGuidEntry = UInt16.MaxValue;
                HtHandleContentionEvent = UInt16.MaxValue;
                OtName = UInt16.MaxValue;
                OtIndex = UInt16.MaxValue;
                ObDecodeShift = UInt16.MaxValue;
                ObAttributesShift = UInt16.MaxValue;
                AlpcCommunicationInfo = UInt16.MaxValue;
                AlpcOwnerProcess = UInt16.MaxValue;
                AlpcConnectionPort = UInt16.MaxValue;
                AlpcServerCommunicationPort = UInt16.MaxValue;
                AlpcClientCommunicationPort = UInt16.MaxValue;
                AlpcHandleTable = UInt16.MaxValue;
                AlpcHandleTableLock = UInt16.MaxValue;
                AlpcAttributes = UInt16.MaxValue;
                AlpcAttributesFlags = UInt16.MaxValue;
                AlpcPortContext = UInt16.MaxValue;
                AlpcPortObjectLock = UInt16.MaxValue;
                AlpcSequenceNo = UInt16.MaxValue;
                AlpcState = UInt16.MaxValue;
                KtReadOperationCount = UInt16.MaxValue;
                KtWriteOperationCount = UInt16.MaxValue;
                KtOtherOperationCount = UInt16.MaxValue;
                KtReadTransferCount = UInt16.MaxValue;
                KtWriteTransferCount = UInt16.MaxValue;
                KtOtherTransferCount = UInt16.MaxValue;
                MmSectionControlArea = UInt16.MaxValue;
                MmControlAreaListHead = UInt16.MaxValue;
                MmControlAreaLock = UInt16.MaxValue;
                EpSectionObject = UInt16.MaxValue;
            }
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct DynFieldsLxcore
        {
            public UInt16 LxPicoProc;
            public UInt16 LxPicoProcInfo;
            public UInt16 LxPicoProcInfoPID;
            public UInt16 LxPicoThrdInfo;
            public UInt16 LxPicoThrdInfoTID;

            public DynFieldsLxcore()
            {
                LxPicoProc = UInt16.MaxValue;
                LxPicoProcInfo = UInt16.MaxValue;
                LxPicoProcInfoPID = UInt16.MaxValue;
                LxPicoThrdInfo = UInt16.MaxValue;
                LxPicoThrdInfoTID = UInt16.MaxValue;
            }
        }

        public static bool Execute(string OutDir, bool StrictChecks)
        {
            string manifestFile = $"{Build.BuildWorkingFolder}\\kphlib\\kphdyn.xml";
            string headerFile = $"{Build.BuildWorkingFolder}\\kphlib\\include\\kphdyn.h";
            string sourceFile = $"{Build.BuildWorkingFolder}\\kphlib\\kphdyn.c";

            // Check for new or modified content. We don't want to touch the file if it's not needed.
            {
                string headerUpdateText = GenerateHeader();
                string headerCurrentText = Utils.ReadAllText(headerFile);

                if (!string.Equals(headerUpdateText, headerCurrentText, StringComparison.OrdinalIgnoreCase))
                {
                    Utils.WriteAllText(headerFile, headerUpdateText);
                }

                Program.PrintColorMessage($"Dynamic header -> {headerFile}", ConsoleColor.Cyan);
            }

            byte[] config = GenerateConfig(manifestFile);

            {
                var headerUpdateText = GenerateSource(BytesToString(config));
                var headerCurrentText = Utils.ReadAllText(sourceFile);

                if (!string.Equals(headerUpdateText, headerCurrentText, StringComparison.OrdinalIgnoreCase))
                {
                    Utils.WriteAllText(sourceFile, headerUpdateText);
                }

                Program.PrintColorMessage($"Dynamic source -> {sourceFile}", ConsoleColor.Cyan);
            }

            if (string.IsNullOrWhiteSpace(OutDir))
                return true;

            string configFile = $"{OutDir}\\ksidyn.bin";

            if (File.Exists(configFile))
            {
                var configUpdateBytes = Utils.ReadAllBytes(configFile).AsSpan();
                var configSigFileName = Path.ChangeExtension(configFile, ".sig");

                if (configUpdateBytes.SequenceEqual(config) && File.Exists(configSigFileName))
                {
                    Program.PrintColorMessage($"Dynamic config -> {configFile}", ConsoleColor.Cyan);
                    return true;
                }

                File.Delete(configFile);
                File.Delete(configSigFileName);
                Utils.WriteAllBytes(configFile, config);

                bool configFileSig = Verify.CreateSigFile("kph", configFile, StrictChecks);

                Program.PrintColorMessage($"Dynamic config -> {configFile}", ConsoleColor.Cyan);

                return configFileSig;
            }
            else
            {
                Directory.CreateDirectory(OutDir);
                Utils.WriteAllBytes(configFile, config);

                bool configFileSig = Verify.CreateSigFile("kph", configFile, StrictChecks);

                Program.PrintColorMessage($"Dynamic config -> {configFile}", ConsoleColor.Cyan);

                return configFileSig;
            }
        }

        private static string GenerateHeader()
        {
            StringBuilder sb = new StringBuilder(8192);

            sb.AppendLine(FileHeader);
            sb.AppendLine();
            sb.AppendLine("#pragma once");
            sb.AppendLine();
            sb.AppendLine(Includes);
            sb.AppendLine(DynConfigC);
            sb.AppendLine();
            sb.AppendLine("#ifdef _WIN64");
            sb.AppendLine("extern CONST BYTE KphDynConfig[];");
            sb.AppendLine("extern CONST ULONG KphDynConfigLength;");
            sb.AppendLine("#endif");

            return sb.ToString();
        }

        private static string GenerateSource(
            string Config
            )
        {
            StringBuilder sb = new StringBuilder(16348);

            sb.AppendLine(FileHeader);
            sb.AppendLine();
            sb.AppendLine(Includes);
            sb.AppendLine();
            sb.AppendLine("#ifdef _WIN64");
            sb.AppendLine("CONST BYTE KphDynConfig[] =");
            sb.AppendLine("{");
            sb.Append(Config);
            sb.AppendLine("};");
            sb.AppendLine();
            sb.AppendLine("CONST ULONG KphDynConfigLength = ARRAYSIZE(KphDynConfig);");
            sb.AppendLine("#endif");

            return sb.ToString();
        }

        private static byte[] GenerateConfig(string ManifestFile)
        {
            var xml = new XmlDocument();
            var fieldsMap = new Dictionary<UInt32, XmlNode>();
            var fieldsOffsets = new Dictionary<UInt32, UInt32>();
            var fieldsStream = new MemoryStream();
            var fieldsWirter = new BinaryWriter(fieldsStream);
            var entries = new List<DynDataEntry>(10);

            xml.Load(ManifestFile);

            var dyn = xml.SelectSingleNode("/dyn");
            var dataNodes = dyn?.SelectNodes("data");
            var fieldsNodes = dyn?.SelectNodes("fields");

            foreach (XmlNode field in fieldsNodes)
            {
                fieldsMap.Add(UInt32.Parse(field.Attributes?.GetNamedItem("id")?.Value), field);
            }

            foreach (XmlNode data in dataNodes)
            {
                var entry = new DynDataEntry();
                var file = data.Attributes?.GetNamedItem("file")?.Value;
                var arch = data.Attributes?.GetNamedItem("arch")?.Value;
                var timestamp = data.Attributes?.GetNamedItem("timestamp")?.Value;
                var size = data.Attributes?.GetNamedItem("size")?.Value;
                var dynClass = ClassFromString(file);

                entry.Class = (UInt16)dynClass;
                entry.Machine = MachineFromString(arch);
                entry.TimeDateStamp = UInt32.Parse(timestamp[2..], NumberStyles.HexNumber);
                entry.SizeOfImage = UInt32.Parse(size[2..], NumberStyles.HexNumber);

                var fieldId = UInt32.Parse(data.InnerText);

                if (!fieldsOffsets.TryGetValue(fieldId, out UInt32 offset))
                {
                    offset = (UInt32)fieldsStream.Length;
                    fieldsOffsets.Add(fieldId, offset);

                    switch (dynClass)
                    {
                        case Class.Ntoskrnl:
                        case Class.Ntkrla57:
                        {
                            var fieldsData = new DynFieldsKernel();
                            foreach (XmlNode field in fieldsMap[fieldId].SelectNodes("field"))
                            {
                                var value = field.Attributes?.GetNamedItem("value")?.Value;
                                var name = field.Attributes?.GetNamedItem("name")?.Value;
                                var member = typeof(DynFieldsKernel).GetField(name);
                                member.SetValueDirect(__makeref(fieldsData), UInt16.Parse(value[2..], NumberStyles.HexNumber));
                            }
                            fieldsWirter.Write(MemoryMarshal.AsBytes(MemoryMarshal.CreateSpan(ref fieldsData, 1)));
                            break;
                        }
                        case Class.Lxcore:
                        {
                            var fieldsData = new DynFieldsLxcore();
                            foreach (XmlNode field in fieldsMap[fieldId].SelectNodes("field"))
                            {
                                var value = field.Attributes?.GetNamedItem("value")?.Value;
                                var name = field.Attributes?.GetNamedItem("name")?.Value;
                                var member = typeof(DynFieldsLxcore).GetField(name);
                                member.SetValueDirect(__makeref(fieldsData), UInt16.Parse(value[2..], NumberStyles.HexNumber));
                            }
                            fieldsWirter.Write(MemoryMarshal.AsBytes(MemoryMarshal.CreateSpan(ref fieldsData, 1)));
                            break;
                        }
                        default:
                        {
                            throw new Exception($"invalid class {dynClass}");
                        }
                    }
                }

                entry.Offset = offset;

                entries.Add(entry);
            }

            using (var stream = new MemoryStream())
            using (var writer = new BinaryWriter(stream))
            {
                //
                // Write the version, session token public key, and count first,
                // then the blocks. This conforms with KPH_DYN_CONFIG.
                //
                writer.Write(Version);
                writer.Write(SessionTokenPublicKey);
                writer.Write((uint)entries.Count);
                writer.Write(MemoryMarshal.AsBytes(CollectionsMarshal.AsSpan(entries)));
                writer.Write(fieldsStream.ToArray());

                return stream.ToArray();
            }
        }

        private static string BytesToString(byte[] Buffer)
        {
            using (MemoryStream stream = new MemoryStream(Buffer, false))
            {
                StringBuilder hex = new StringBuilder(64);
                StringBuilder sb = new StringBuilder(8192);
                Span<byte> bytes = stackalloc byte[8];

                while (true)
                {
                    var len = stream.Read(bytes);

                    if (len == 0)
                    {
                        break;
                    }

                    for (int i = 0; i < len; i++)
                    {
                        hex.AppendFormat("0x{0:x2}, ", bytes[i]);
                    }
                    hex.Remove(hex.Length - 1, 1);

                    sb.Append("    ");
                    sb.AppendLine(hex.ToString());
                    hex.Clear();

                    if (len < bytes.Length)
                    {
                        break;
                    }
                }

                return sb.ToString();
            }
        }
    }
}
