#ifndef EMBEDDED_ART_PROVIDER_HPP
#define EMBEDDED_ART_PROVIDER_HPP

#include <wx/artprov.h>
#include <wx/bitmap.h>
#include <wx/image.h>

struct EmbeddedIconData;

class EmbeddedArtProvider : public wxArtProvider
{
public:
    EmbeddedArtProvider();

protected:
    wxBitmap CreateBitmap(const wxArtID& id, const wxArtClient& client, const wxSize& size) override;

private:
/*
    struct IconData {
        const unsigned char* data;
        size_t size;
        int width;
        int height;
    };
*/

    const EmbeddedIconData* FindBestIcon(const wxString& name, const wxSize& requestedSize) const;
    wxBitmap LoadPngIcon(const EmbeddedIconData* iconData, const wxSize& size) const;
};

#endif
