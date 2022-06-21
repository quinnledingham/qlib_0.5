#ifndef WIN32_CLIPBOARD_H
#define WIN32_CLIPBOARD_H

UINT uFormat = (UINT)(-1); 
BOOL fAuto = TRUE; 

void WINAPI SetAutoView(HWND hwnd) 
{ 
    static UINT auPriorityList[] = { 
        CF_OWNERDISPLAY, 
        CF_TEXT, 
        CF_ENHMETAFILE, 
        CF_BITMAP 
    }; 
    
    uFormat = GetPriorityClipboardFormat(auPriorityList, 4); 
    fAuto = TRUE; 
    
    InvalidateRect(hwnd, NULL, TRUE); 
    UpdateWindow(hwnd); 
} 

void WINAPI InitMenu(HWND hwnd, HMENU hmenu) 
{ 
    UINT uFormat; 
    char szFormatName[80]; 
    LPCSTR lpFormatName; 
    UINT fuFlags; 
    UINT idMenuItem; 
    
    // If a menu is not the display menu, no initialization is necessary. 
    
    if (GetMenuItemID(hmenu, 0) != IDM_AUTO) 
        return; 
    
    // Delete all menu items except the first. 
    
    while (GetMenuItemCount(hmenu) > 1) 
        DeleteMenu(hmenu, 1, MF_BYPOSITION); 
    
    // Check or uncheck the Auto menu item. 
    
    fuFlags = fAuto ? MF_BYCOMMAND | MF_CHECKED : MF_BYCOMMAND | MF_UNCHECKED; 
    CheckMenuItem(hmenu, IDM_AUTO, fuFlags); 
    
    // If there are no clipboard formats, return. 
    
    if (CountClipboardFormats() == 0) 
        return; 
    
    // Open the clipboard. 
    
    if (!OpenClipboard(hwnd)) 
        return; 
    
    // Add a separator and then a menu item for each format. 
    
    AppendMenu(hmenu, MF_SEPARATOR, 0, NULL); 
    uFormat = EnumClipboardFormats(0); 
    
    while (uFormat) 
    { 
        // Call an application-defined function to get the name 
        // of the clipboard format. 
        
        lpFormatName = GetPredefinedClipboardFormatName(uFormat); 
        
        // For registered formats, get the registered name. 
        
        if (lpFormatName == NULL) 
        {
            
            // Note that, if the format name is larger than the
            // buffer, it is truncated. 
            if (GetClipboardFormatName(uFormat, szFormatName, 
                                       sizeof(szFormatName))) 
                lpFormatName = szFormatName; 
            else 
                lpFormatName = "(unknown)"; 
        } 
        
        // Add a menu item for the format. For displayable 
        // formats, use the format ID for the menu ID. 
        
        if (IsDisplayableFormat(uFormat)) 
        { 
            fuFlags = MF_STRING; 
            idMenuItem = uFormat; 
        } 
        else 
        { 
            fuFlags = MF_STRING | MF_GRAYED; 
            idMenuItem = 0; 
        } 
        AppendMenu(hmenu, fuFlags, idMenuItem, lpFormatName); 
        
        uFormat = EnumClipboardFormats(uFormat); 
    } 
    CloseClipboard(); 
    
} 

BOOL WINAPI IsDisplayableFormat(UINT uFormat) 
{ 
    switch (uFormat) 
    { 
        case CF_OWNERDISPLAY: 
        case CF_TEXT: 
        case CF_ENHMETAFILE: 
        case CF_BITMAP: 
        return TRUE; 
    } 
    return FALSE; 
}

#endif //WIN32_CLIPBOARD_H
