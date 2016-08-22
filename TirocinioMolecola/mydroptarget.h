#ifndef MYDROPTARGET_H
#define MYDROPTARGET_H

#endif // MYDROPTARGET_H


#include <shobjidl.h>
#include <shellapi.h>
#include <iostream>
#include <preferences.h>

using namespace std;


class MyDropTarget : public IDropTarget{

public:

    string droppedFilePath;

    STDMETHODIMP DragEnter(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect);
    STDMETHODIMP DragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect);
    STDMETHODIMP DragLeave();
    STDMETHODIMP Drop(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect);


    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject);
    ULONG STDMETHODCALLTYPE AddRef(void);
    ULONG STDMETHODCALLTYPE Release(void);

    bool checkPDBFromPath(const char *path);



};
