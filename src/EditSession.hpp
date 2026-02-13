#pragma once

class EditSession : public ITfEditSession {
public:
  EditSession(ITfContext* pContext, std::wstring const& str) : fContext(pContext), fRefCount(1), fString(str) {
    fContext->AddRef();
  }

  ~EditSession() {
    fContext->Release();
  }

  STDMETHODIMP QueryInterface(REFIID riid, _Outptr_ void** ppvObj) override {
    return E_NOINTERFACE;
  }

  STDMETHODIMP_(ULONG) AddRef() override {
    return ++fRefCount;
  }

  STDMETHODIMP_(ULONG) Release() override {
    LONG cr = --fRefCount;
    assert(fRefCount >= 0);
    if (fRefCount == 0) {
      delete this;
    }
    return cr;
  }

  STDMETHODIMP DoEditSession(TfEditCookie ec) override {
    ULONG fetched = 0;
    TF_SELECTION selection;
    if (fContext->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &selection, &fetched) != S_OK) {
      return S_FALSE;
    }
    defer{
      selection.range->Release();
    };
    if (selection.range->SetText(ec, 0, fString.c_str(), (LONG)fString.size()) != S_OK) {
      return S_FALSE;
    }
    selection.range->Collapse(ec, TF_ANCHOR_END);
    fContext->SetSelection(ec, 1, &selection);
    return S_OK;
  }

private:
  ITfContext* fContext;
  LONG fRefCount;
  std::wstring fString;
};
