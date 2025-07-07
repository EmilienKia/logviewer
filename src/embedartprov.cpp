#include "embedartprov.hpp"
#include "embedded_resources.h"
#include <wx/mstream.h>
#include <wx/image.h>
#include <algorithm>
#include <cmath>

EmbeddedArtProvider::EmbeddedArtProvider()
{
}

wxBitmap EmbeddedArtProvider::CreateBitmap(const wxArtID& id, const wxArtClient& client, const wxSize& size)
{
    wxSize requestedSize = size;
    if (requestedSize == wxDefaultSize) {
        requestedSize = wxSize(16, 16); // Taille par défaut
    }

    const EmbeddedIconData* iconData = FindBestIcon(id, requestedSize);
    if (!iconData) {
        return wxNullBitmap;
    }

    return LoadPngIcon(iconData, requestedSize);
}

const EmbeddedIconData* EmbeddedArtProvider::FindBestIcon(const wxString& name, const wxSize& requestedSize) const
{
    auto& icons = GetEmbeddedIcons();
    auto it = icons.find(name.ToStdString());
    if (it == icons.end()) {
        return nullptr;
    }

    const auto& iconList = it->second;
    if (iconList.empty()) {
        return nullptr;
    }

    const EmbeddedIconData* bestIcon = nullptr;
    int bestScore = INT_MAX;

    for (const auto& icon : iconList) {
        // Correspondance exacte
        if (icon.width == requestedSize.x && icon.height == requestedSize.y) {
            return &icon;
        }

        // Calculer le score basé sur la différence de taille
        int sizeDiff = std::abs(icon.width - requestedSize.x) + std::abs(icon.height - requestedSize.y);

        // Préférer les tailles plus grandes pour éviter la perte de qualité lors du redimensionnement
        if (icon.width >= requestedSize.x && icon.height >= requestedSize.y) {
            sizeDiff = sizeDiff / 2; // Bonus pour les tailles plus grandes
        } else {
            sizeDiff = sizeDiff * 2; // Pénalité pour les tailles plus petites
        }

        if (sizeDiff < bestScore) {
            bestIcon = &icon;
            bestScore = sizeDiff;
        }
    }

    return bestIcon;
}

wxBitmap EmbeddedArtProvider::LoadPngIcon(const EmbeddedIconData* iconData, const wxSize& size) const
{
    wxMemoryInputStream stream(iconData->data, iconData->size);
    wxImage image(stream, wxBITMAP_TYPE_PNG);

    if (image.IsOk()) {
        // Redimensionner si nécessaire
        if (size != wxSize(image.GetWidth(), image.GetHeight())) {
            image = image.Scale(size.x, size.y, wxIMAGE_QUALITY_HIGH);
        }
        return wxBitmap(image);
    }

    return wxNullBitmap;
}
