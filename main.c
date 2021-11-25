#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <errno.h>

#include "picohash.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#define BUFSIZE 1024
#define INSTALL_FILENAME "HashCalc.exe"
#define INSTALL_FOLDER "HashCalc"
#define REGISTRY_INSTALL_NAME "hash_calc"
#define DISPLAY_NAME "File Hash Calculator"
#define DISPLAY_VERSION "1.0.0"
#define PUBLISHER "Asaf Zanjiri"

/* This function would display verbose help message in case no arguments were used. */
void FirstUse() {
    printf("File Hash Calculator | Asaf Zan\n");
    printf("This program needs to be run with command line arguments. The valid arguments are:\n");
    printf("1.'--install'\n");
    printf("2.'--calc-hash'\n");
    printf("3.'--uninstall'\n");
    printf("Press any key to exit...\n");
    getchar();
}

/* This function would create the sub-folder/s keys inside a given registry path. Exits program if fails. */
void CreateRegKeyPath(HKEY *hKey, HKEY type, const LPCSTR path) {
    LONG lReg = RegCreateKeyExA(type, path, 0, NULL,
                                REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS | KEY_WOW64_64KEY, NULL, hKey, NULL);
    if (lReg != ERROR_SUCCESS) {
        printf("Registry creation failed!\nWindows error code: %lu\nPress any key to exit...\n", lReg);
        RegCloseKey(*hKey); // Closes the registry handle.
        getchar();
        exit(lReg);
    }
    printf("Registry path \"%s\" successfully created...\n", path);
}

/* This function would set a key and a value in a given registry path. Exits program if fails. */
void SetRegKeyValue(HKEY hKey, const LPCSTR key, const char *value) {
    LONG lReg = RegSetValueExA(hKey, key, (DWORD) NULL, REG_SZ, (CONST BYTE *) value, strlen(value));
    if (lReg != ERROR_SUCCESS) {
        printf("Registry set failed!\nWindows error code: %lu\nPress any key to exit...\n", lReg);
        RegCloseKey(hKey); // Closes the registry handle.
        getchar();
        exit(lReg);
    }
    printf("Registry key successfully added with a value\n");
}


/* This function deletes a sub-key and its values from the given registry path. Exists program if fails. */
void DeleteRegKey(HKEY type, const LPCSTR path) {
    LONG lReg = RegDeleteKeyExA(type, path, KEY_ALL_ACCESS | KEY_WOW64_64KEY, 0);
    if (lReg != ERROR_SUCCESS) {
        if (lReg == 2L) printf("Registry key doesn't exist. Continuing the uninstallation process.\n");
        else {
            printf("Registry key deletion failed!\nWindows error code: %lu\nPress any key to exit...\n", lReg);
            getchar();
            exit(lReg);
        }
    }
    printf("Registry path \"%s\" successfully deleted...\n", path);
}


/* Makes a copy of the EXE inside the given installation folder/filename. Exits program if fails. */
const char *CreateExecutable() {
    // Initialize variables.
    char *installPath = _strdup(getenv("ProgramFiles"));
    sprintf(installPath, "%s\\%s", installPath, INSTALL_FOLDER);
    BOOL bDir = CreateDirectoryA(installPath, NULL);

    // Create install folder in %ProgramFiles%
    if (bDir == 0) {
        switch (GetLastError()) {
            case ERROR_PATH_NOT_FOUND:
                printf("Error creating program files directory!\nOne or more intermediate directories do not exist; this function will only create the final directory in the path.\n");
                exit(ERROR_PATH_NOT_FOUND);
            case ERROR_ALREADY_EXISTS:
                printf("Path already exists. Continuing with the installation.\n");
                break;
        }
    } else printf("Successfully created program files directory.\n");
    sprintf(installPath, "%s\\%s", installPath, INSTALL_FILENAME);


    // Create an empty EXE file for the program.
    FILE *fp = fopen(installPath, "w");
    if (fp == NULL) {
        fprintf(stderr, "Error %d - %s", errno, strerror(errno));
        exit(errno);
    }
    fclose(fp);
    printf("Successfully created EXE file in the installation directory.\n");


    // Copy this EXE contents to the empty EXE in the installation path.
    char filename[MAX_PATH];
    BOOL stats = 0;
    DWORD size = GetModuleFileNameA(NULL, filename, MAX_PATH);
    if (size) {
        CopyFile(filename, installPath, stats);
        printf("Successfully copied the file contents to the file in the installation path.\n");
    } else {
        printf("Could not find EXE file name.\n");
        exit(-1);
    }
    return installPath;
}


/* Deletes the %ProgramFiles& install path as well as its contents then exits the program. */
void SelfDelete() {
    // Initialize variables.
    char *installPath = _strdup(getenv("ProgramFiles"));
    sprintf(installPath, "%s\\%s", installPath, INSTALL_FOLDER);
    char cmd[MAX_PATH];
    sprintf(cmd, "cmd.exe /c start /MIN timeout /T 4 /NOBREAK > Nul & echo Uninstalling HashCalc files... & rmdir \"%s\" /S /Q", installPath);

    WinExec(cmd, SW_HIDE); // Runs the command to delete the files hidden.
    exit(0);
}


int main(int argc, char *argv[]) {
    // If no arguments were inputted display the default help message.
    if (argc < 2) {
        FirstUse();
        return 0;
    }

    if (!strcmp(argv[1], "--install")) {
        printf("Note: If the installation fails, make sure you're running the program with admin permissions.\nStarting installation...\n");

        // Initialize variables.
        HKEY hKey;
        char installPath[MAX_PATH];
        strcpy_s(installPath, MAX_PATH, CreateExecutable());
        char tmpInstallPath[MAX_PATH];
        strcpy_s(tmpInstallPath, MAX_PATH, installPath);
        char regInstallPath[MAX_PATH];
        strcpy_s(regInstallPath, MAX_PATH, "*\\shell");

        // Set right click name.
        sprintf(regInstallPath, "%s\\%s", regInstallPath, REGISTRY_INSTALL_NAME);
        CreateRegKeyPath(&hKey, HKEY_CLASSES_ROOT, (LPCSTR) regInstallPath);
        SetRegKeyValue(hKey, NULL, "Calculate file hash");
        RegCloseKey(hKey); // Closes the registry handle

        // Set command to execute when pressing the button.
        sprintf(regInstallPath, "%s%s", regInstallPath, "\\command");
        CreateRegKeyPath(&hKey, HKEY_CLASSES_ROOT, (LPCSTR) regInstallPath);
        sprintf(installPath, "%s%s", installPath, " --calc-hash \"%L\"");
        SetRegKeyValue(hKey, NULL, installPath);
        RegCloseKey(hKey); // Closes the registry handle.

        //Create uninstaller.
        printf("Creating uninstaller...\n");
        strcpy(regInstallPath, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall");
        sprintf(regInstallPath, "%s\\%s", regInstallPath, REGISTRY_INSTALL_NAME);
        CreateRegKeyPath(&hKey, HKEY_LOCAL_MACHINE, (LPCSTR) regInstallPath);
        SetRegKeyValue(hKey, "DisplayName", DISPLAY_NAME);
        SetRegKeyValue(hKey, "DisplayVersion", DISPLAY_VERSION);
        SetRegKeyValue(hKey, "Publisher", PUBLISHER);
        sprintf(tmpInstallPath, "%s%s", tmpInstallPath, " --uninstall");
        SetRegKeyValue(hKey, "UninstallString", tmpInstallPath);
        RegCloseKey(hKey); // Closes the registry handle.

        printf("Successfully finished installing the program!\n");

    }
    else if (!strcmp(argv[1], "--calc-hash")) {
        printf("Calculating hash...\n");
        // Turn the argument path to LPWSTR to support unicoded paths.
        LPWSTR *szArglist;
        szArglist = CommandLineToArgvW(GetCommandLineW(), &argc);
        if (NULL == szArglist) {
            wprintf(L"CommandLineToArgvW failed\n");
            return 0;
        }

        // Try opening the file.
        FILE *fp = _wfopen(szArglist[2], L"rb");
        if (fp == NULL) {
            fprintf(stderr, "Error %d - %s\nPress any key to exit...", errno, strerror(errno));
            getchar();
            exit(errno);
        }

        // Free memory allocated for CommandLineToArgvW arguments.
        LocalFree(szArglist);

        // General vars
        unsigned int bytes;
        char data[BUFSIZE];

        //MD5 vars
        picohash_ctx_t md5_ctx;
        unsigned char md5_digest[PICOHASH_MD5_DIGEST_LENGTH];
        char md5_hash[sizeof md5_digest * 2 + 1];

        //SHA256 vars
        picohash_ctx_t sha256_ctx;
        unsigned char sha256_digest[PICOHASH_SHA256_DIGEST_LENGTH];
        char sha256_hash[sizeof sha256_digest * 2 + 1];

        // Calculate digest value
        picohash_init_md5(&md5_ctx);
        picohash_init_sha256(&sha256_ctx);
        while ((bytes = fread(data, 1, BUFSIZE, fp)) != 0) {
            picohash_update(&md5_ctx, data, bytes);
            picohash_update(&sha256_ctx, data, bytes);
        }
        fclose(fp); // Close file handler
        picohash_final(&md5_ctx, md5_digest);
        picohash_final(&sha256_ctx, sha256_digest);


        // Turn MD5 digest to string hash
        for (int i = 0, j = 0; i < PICOHASH_MD5_DIGEST_LENGTH; i++, j += 2)
            sprintf(md5_hash + j, "%02x", md5_digest[i]);
        md5_hash[sizeof md5_digest * 2] = 0;

        // Turn SHA256 digest to string hash
        for (int i = 0, j = 0; i < PICOHASH_SHA256_DIGEST_LENGTH; i++, j += 2)
            sprintf(sha256_hash + j, "%02x", sha256_digest[i]);
        sha256_hash[sizeof sha256_digest * 2] = 0;

        // Print hashes
        printf("Hashes of: %s\n", argv[2]);
        printf("MD5 - %s\n", md5_hash);
        printf("SHA256 - %s\n", sha256_hash);
        printf("Press any key to exit...\n");
        getchar();
    }
    else if (!strcmp(argv[1], "--uninstall")) {
        printf("Uninstalling program...\n");
        char regInstallPath[MAX_PATH];

        // Deletes the right click command registry.
        strcpy_s(regInstallPath, MAX_PATH, "*\\shell");
        sprintf(regInstallPath, "%s\\%s\\command", regInstallPath, REGISTRY_INSTALL_NAME);
        DeleteRegKey(HKEY_CLASSES_ROOT, regInstallPath);

        // Deletes the right click registry.
        strcpy_s(regInstallPath, MAX_PATH, "*\\shell");
        sprintf(regInstallPath, "%s\\%s", regInstallPath, REGISTRY_INSTALL_NAME);
        DeleteRegKey(HKEY_CLASSES_ROOT, regInstallPath);

        // Deletes the uninstaller registry.
        strcpy_s(regInstallPath, MAX_PATH, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall");
        sprintf(regInstallPath, "%s\\%s", regInstallPath, REGISTRY_INSTALL_NAME);
        DeleteRegKey(HKEY_LOCAL_MACHINE, regInstallPath);

        // Delete the %ProgramFiles% path
        SelfDelete();

    }
    else {
        printf("Invalid option!\nPlease open the program without any arguments to see all valid options.\nPress any key to exit...\n");
        getchar();
    }

    return 0;
}