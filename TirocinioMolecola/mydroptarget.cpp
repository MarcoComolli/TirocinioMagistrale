#include <mydroptarget.h>
#include <quterenderer.h>


void initComponents(Preferences& prefs, bool isQuteRend);
void initQuteRenderer(bool first);

extern QuteRenderer quteRenderer;

STDMETHODIMP MyDropTarget::DragEnter(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect){
    std::cout << " DragEnter " << std::endl;
    *pdwEffect = DROPEFFECT_MOVE;
    FORMATETC fdrop = {CF_HDROP, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    STGMEDIUM stgmed;

    if(pDataObj->GetData(&fdrop, &stgmed) == S_OK){
        HDROP hdrop = (HDROP)GlobalLock(stgmed.hGlobal);
        UINT nCnt  = DragQueryFile(hdrop, (UINT)-1, NULL, 0);
        if(nCnt == 1){
            for(int nIndex = 0; nIndex < nCnt; ++nIndex) {
                UINT nSize = DragQueryFile(hdrop, nIndex, NULL, 0);
                TCHAR *pszFileName = new TCHAR[++nSize];
                if(DragQueryFile(hdrop, nIndex, pszFileName, nSize)) {
                    string fileName(pszFileName);
                    droppedFilePath = fileName;
                    std::cout << "File: " << fileName << std::endl;
                }
                delete [] pszFileName;
            }
            DragFinish(hdrop);

            if(checkPDBFromPath(droppedFilePath.c_str())){
                return S_OK;
            }
            else{
                return E_INVALIDARG;
            }

        }
        else{
            std::cout << "Questo e' un bel problema " << std::endl;
            std::cout << "Troppi files! =( " << std::endl;
            return E_INVALIDARG;
        }
    }
    else{
        return E_INVALIDARG;
    }
}

STDMETHODIMP MyDropTarget::DragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect){
    return S_OK;
}

STDMETHODIMP MyDropTarget::DragLeave(){
    return S_OK;
}


STDMETHODIMP MyDropTarget::Drop(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect){
    std::cout << " Drop " << droppedFilePath << std::endl;
    const char * pDropped =  droppedFilePath.c_str();
    if(checkPDBFromPath(pDropped)){
        delete pDropped;
        quteRenderer.ds.prefs.molPath = wstring(droppedFilePath.begin(),droppedFilePath.end());
        initComponents(quteRenderer.ds.prefs, true);
        initQuteRenderer(false);
        return S_OK;
    }
    else{
        delete pDropped;
         return E_INVALIDARG;
    }

}

HRESULT STDMETHODCALLTYPE MyDropTarget::QueryInterface(REFIID riid, void **ppvObject){
    return E_NOTIMPL;
}
ULONG STDMETHODCALLTYPE MyDropTarget::AddRef(void){
    return 0;
}
ULONG STDMETHODCALLTYPE MyDropTarget::Release(void){
    return 0;
}

bool MyDropTarget::checkPDBFromPath(const char * path){

    char drive[_MAX_DRIVE];
    char dir[_MAX_DIR];
    char fname[_MAX_FNAME];
    char ext[_MAX_EXT];
    _splitpath_s(path,drive,dir,fname,ext);

    if(string(ext) == ".pdb"){
         return true;
    }
    else{
        return false;
    }
}

