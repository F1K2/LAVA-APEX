#pragma once
#include <Windows.h>
#include <cstdint>
#include "Memory.hpp"

// ============================================
// CLIENT ENCRYPTÉ POUR COMMUNIQUER AVEC LE DRIVER
// ============================================

namespace encrypted_client {
    // Classe pour gérer les communications chiffrées avec le driver
    class EncryptedDriverClient {
    private:
        HANDLE driverHandle;
        ULONG64 sessionId;
        bool sessionActive;

        // Clé de chiffrement
        UCHAR encryptionKey[32];
        ULONG keySize;

        // Générer une clé aléatoire
        void GenerateKey() {
            LARGE_INTEGER perf;
            QueryPerformanceCounter(&perf);
            for (ULONG i = 0; i < 32; ++i) {
                encryptionKey[i] = (UCHAR)((perf.QuadPart >> (i * 8)) ^ (i * 0x37));
            }
            keySize = 32;
        }

        // Chiffrement XOR simple
        void XorEncryptDecrypt(PUCHAR data, SIZE_T size) {
            for (SIZE_T i = 0; i < size; ++i) {
                data[i] ^= encryptionKey[i % keySize];
            }
        }

    public:
        EncryptedDriverClient() : driverHandle(INVALID_HANDLE_VALUE),
            sessionId(0), sessionActive(false), keySize(32) {
            GenerateKey();
        }

        ~EncryptedDriverClient() {
            if (driverHandle != INVALID_HANDLE_VALUE) {
                CloseHandle(driverHandle);
            }
        }

        // Se connecter au driver
        bool Connect() {
            driverHandle = CreateFileW(
                L"\\\\.\\{A7F3B2C1-9D4E-8F6A-7B5C-3E2D1A0F9E8D}",
                GENERIC_READ | GENERIC_WRITE,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                NULL,
                OPEN_EXISTING,
                0,
                NULL
            );

            if (driverHandle == INVALID_HANDLE_VALUE) {
                return false;
            }

            // Créer une session de chiffrement
            return CreateSession();
        }

        // Créer une session de chiffrement avec le driver
        bool CreateSession() {
            typedef struct _CREATE_SESSION_REQUEST {
                INT32 security;
                UCHAR key[32];
                ULONG keySize;
            } CREATE_SESSION_REQUEST;

            typedef struct _CREATE_SESSION_RESPONSE {
                ULONG64 sessionId;
                NTSTATUS status;
            } CREATE_SESSION_RESPONSE;

            CREATE_SESSION_REQUEST req = { 0 };
            req.security = 0x9F8E7D6C; // CODE_SECURITY
            memcpy(req.key, encryptionKey, 32);
            req.keySize = keySize;

            CREATE_SESSION_RESPONSE resp = { 0 };
            DWORD bytesReturned = 0;

            // CODE_CREATE_SESSION = 0x6E9B2
            ULONG ioctlCode = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x6E9B2, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);

            if (DeviceIoControl(
                driverHandle,
                ioctlCode,
                &req,
                sizeof(req),
                &resp,
                sizeof(resp),
                &bytesReturned,
                NULL
            )) {
                if (resp.status == 0) { // STATUS_SUCCESS
                    sessionId = resp.sessionId;
                    sessionActive = true;
                    return true;
                }
            }

            return false;
        }

        // Lire de la mémoire avec chiffrement
        bool ReadMemory(ULONG processId, ULONG64 address, PVOID buffer, SIZE_T size) {
            if (!sessionActive) {
                return false;
            }

            // Créer la requête RW
            mem::_rw rwReq = { 0 };
            rwReq.security = 0x9F8E7D6C;
            rwReq.process_id = processId;
            rwReq.address = address;
            rwReq.buffer = (ULONG64)buffer;
            rwReq.size = size;
            rwReq.write = FALSE;

            // Chiffrer la requête
            XorEncryptDecrypt((PUCHAR)&rwReq, sizeof(rwReq));

            // Envoyer la requête chiffrée
            // CODE_ENCRYPTED_RW = 0x7F8A9
            ULONG ioctlCode = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x7F8A9, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);

            DWORD bytesReturned = 0;
            return DeviceIoControl(
                driverHandle,
                ioctlCode,
                &rwReq,
                sizeof(rwReq),
                buffer,
                (DWORD)size,
                &bytesReturned,
                NULL
            ) != FALSE;
        }

        // Écrire en mémoire avec chiffrement
        bool WriteMemory(ULONG processId, ULONG64 address, PVOID buffer, SIZE_T size) {
            if (!sessionActive) {
                return false;
            }

            // Créer la requête RW
            mem::_rw rwReq = { 0 };
            rwReq.security = 0x9F8E7D6C;
            rwReq.process_id = processId;
            rwReq.address = address;
            rwReq.buffer = (ULONG64)buffer;
            rwReq.size = size;
            rwReq.write = TRUE;

            // Chiffrer la requête
            XorEncryptDecrypt((PUCHAR)&rwReq, sizeof(rwReq));

            // Envoyer la requête chiffrée
            ULONG ioctlCode = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x7F8A9, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);

            DWORD bytesReturned = 0;
            return DeviceIoControl(
                driverHandle,
                ioctlCode,
                &rwReq,
                sizeof(rwReq),
                NULL,
                0,
                &bytesReturned,
                NULL
            ) != FALSE;
        }
    };
}

// Exemple d'utilisation:
/*
    encrypted_client::EncryptedDriverClient client;
    if (client.Connect()) {
        BYTE buffer[1024];
        if (client.ReadMemory(1234, 0x400000, buffer, sizeof(buffer))) {
            // Données lues avec succès
        }
    }
*/

