#pragma once

class EditSession : public ITfEditSession {
public:
  EditSession(ITfContext* pContext, std::vector<WCHAR> const& str) : fContext(pContext), fRefCount(1), fString(str) {
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
    if (selection.range->SetText(ec, 0, fString.data(), fString.size()) != S_OK) {
      selection.range->Release();
      return S_FALSE;
    }
    selection.range->Collapse(ec, TF_ANCHOR_END);
    fContext->SetSelection(ec, 1, &selection);
    selection.range->Release();
    return S_OK;
  }

private:
  ITfContext* fContext;
  LONG fRefCount;
  std::vector<WCHAR> fString;
};
