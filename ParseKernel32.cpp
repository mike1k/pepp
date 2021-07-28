#include "pepp/PELibrary.hpp"

int main()
{
    pepp::Image64 kernel32("C:\\Windows\\System32\\kernel32.dll");

    if (kernel32.magic() != IMAGE_DOS_SIGNATURE)
        return 1;

    printf("[+] Printing kernel32 sections..\n");

    for (int i = 0; i < kernel32.GetNumberOfSections(); ++i)
    {
        pepp::SectionHeader const& sec = kernel32.GetSectionHeader(i);

        printf("%s\t\t[va: 0x%08X]\t\t[raw: 0x%08X]\t\t[size: 0x%08X]\n",
            sec.GetName().c_str(),
            sec.GetVirtualAddress(),
            sec.GetPointerToRawData(),
            sec.GetVirtualSize());
    }

    printf("[+] Printing data directories..");


    for (int i = 0; i < kernel32.GetPEHeader().DirectoryCount(); ++i)
    {
        IMAGE_DATA_DIRECTORY const& dir = kernel32.GetPEHeader().GetOptionalHeader().GetDataDirectory(i);

        printf("DIRECTORY %02d:\t0x%08X\t\t\t[Size: 0x%08X]\n", i, dir.VirtualAddress, dir.Size);
    }

    printf("[+] Printing kernel32 imports..\n");

    kernel32.GetImportDirectory().TraverseImports([](pepp::ModuleImportData_t* imp)
        {
            if (imp->ordinal)
            {
                printf("* %s!%016x\n", imp->module_name.c_str(), std::get<uint64_t>(imp->import_variant));
            }
            else
            {
                printf("* %s!%s\n", imp->module_name.c_str(), std::get<std::string>(imp->import_variant).c_str());
            }
        });

    printf("[+] Printing kernel32 exports..\n");

    kernel32.GetExportDirectory().TraverseExports([](pepp::ExportData_t* exp)
        {
            printf("* %s\n", exp->name.c_str());
        });

    return 0;
}
