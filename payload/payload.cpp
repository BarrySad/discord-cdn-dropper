/*
 * Dummy payload for demonstrating the Discord CDN dropper pipeline.
 *
 * This is intentionally benign — a simple MessageBox to confirm successful
 * delivery and execution. Compile this, run it through the chunker, upload
 * the fractions to Discord, and paste the CDN links into the dropper source
 * to reproduce the full delivery chain end to end.
 */
 
#include <windows.h>
 
int main(VOID)
{
    MessageBoxW(NULL, L"Delivered by DiscordCDN", L"Oooo Scary", MB_OK);
 
    return ERROR_SUCCESS;
}
 
