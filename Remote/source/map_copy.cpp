#include "map_copy.h"
#include "lake_storage.h"
#include <string.h>
static File inFile,outFile;
static String currentDestination;
static uint32_t expectedCopyBytes=0,currentCopyBytes=0;
bool mapCopy_active(){return !!inFile;}
void mapCopy_cancel(){if(inFile)inFile.close();if(outFile)outFile.close();}
bool mapCopy_start(const char *src,const char *dst,size_t maxSize,size_t exactSize)
{
    mapCopy_cancel();
    if(!src||!dst||strlen(dst)>=100||!strcmp(src,dst))return false;
    inFile = SD.open(src, FILE_READ);
    if (!inFile) return false;
    const size_t sourceSize = inFile.size();

    if (!sourceSize || sourceSize > maxSize) { inFile.close(); return false; }
    expectedCopyBytes = static_cast<uint32_t>(sourceSize);
    currentCopyBytes = 0;
    if (!expectedCopyBytes || (exactSize && expectedCopyBytes != exactSize))
    {
        inFile.close();
        return false;
    }
    char dir[100];
    strncpy(dir, dst, sizeof(dir) - 1);
    dir[sizeof(dir) - 1] = '\0';
    char *last = strrchr(dir, '/');
    if (last)
    {
        *last = '\0';
        if (!lakeStorage_makeDirectories(dir)) { inFile.close(); return false; }
    }
    if(SD.exists(dst) && !SD.remove(dst)){inFile.close();return false;}
    currentDestination=dst;
    outFile = SD.open(dst, FILE_WRITE);
    if (!outFile) { inFile.close(); return false; }
    return true;
}
// -1 means failed/short read/write, 0 means copy is ongoing, 1 means exact-size file complete.
int mapCopy_step(uint32_t &copied,uint32_t totalBytes)
{
    if (!inFile || !outFile) {mapCopy_cancel();return -1;}
    uint8_t buffer[4096];
    const size_t n = inFile.read(buffer, sizeof(buffer));
    if (n)
    {
        if (currentCopyBytes > expectedCopyBytes || n > expectedCopyBytes - currentCopyBytes ||
            copied > totalBytes || n > totalBytes - copied)
        {
            inFile.close(); outFile.close(); return -1;
        }
        if (outFile.write(buffer, n) != n)
        {
            inFile.close(); outFile.close();
            return -1;
        }
        currentCopyBytes += (uint32_t)n;
        copied += (uint32_t)n;
    }
    if (!inFile.available())
    {
        outFile.flush(); inFile.close(); outFile.close();
        File verify=SD.open(currentDestination.c_str(),FILE_READ);
        bool ok=verify && verify.size()==expectedCopyBytes && currentCopyBytes==expectedCopyBytes;
        if(verify)verify.close();
        return ok?1:-1;
    }
    if (!n) {mapCopy_cancel();return -1;}
    return 0;
}
