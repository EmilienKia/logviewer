function(embed_resources TARGET_NAME RESOURCE_DIR OUTPUT_FILE)
    file(GLOB_RECURSE PNG_FILES "${RESOURCE_DIR}/*/*.png")

    set(RESOURCE_CODE "#include <wx/wx.h>\n#include <unordered_map>\n#include <vector>\n\n")
    set(RESOURCE_REGISTRY "")

    # Structure pour stocker les informations d'icône PNG
    set(RESOURCE_CODE "${RESOURCE_CODE}struct EmbeddedIconData {\n")
    set(RESOURCE_CODE "${RESOURCE_CODE}    const unsigned char* data;\n")
    set(RESOURCE_CODE "${RESOURCE_CODE}    size_t size;\n")
    set(RESOURCE_CODE "${RESOURCE_CODE}    int width;\n")
    set(RESOURCE_CODE "${RESOURCE_CODE}    int height;\n")
    set(RESOURCE_CODE "${RESOURCE_CODE}};\n\n")

    foreach(PNG_FILE ${PNG_FILES})
        # Extraire les informations du chemin
        file(RELATIVE_PATH REL_PATH ${RESOURCE_DIR} ${PNG_FILE})
        get_filename_component(SIZE_GROUP_DIR ${REL_PATH} DIRECTORY)
        get_filename_component(SIZE_NAME ${SIZE_GROUP_DIR} DIRECTORY)
        get_filename_component(GROUP_NAME ${SIZE_GROUP_DIR} NAME)
        get_filename_component(ICON_NAME ${PNG_FILE} NAME_WE)
        message(DEBUG "Processing icon resource file: ${REL_PATH}")
        message(DEBUG "Size name: ${SIZE_NAME}, group name: ${GROUP_NAME}, Icon: ${ICON_NAME}")

        # Ignorer le répertoire "scalable" s'il existe
        if(${SIZE_NAME} STREQUAL "scalable")
            continue()
        endif()

        # Créer un identifiant unique
        string(MAKE_C_IDENTIFIER "${SIZE_NAME}_${ICON_NAME}" RESOURCE_VAR)

        # Lire le fichier en hexadécimal
        file(READ ${PNG_FILE} HEX_DATA HEX)
        string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," HEX_ARRAY ${HEX_DATA})
        string(REGEX REPLACE ",$" "" HEX_ARRAY ${HEX_ARRAY})

        # Générer les données
        set(RESOURCE_CODE "${RESOURCE_CODE}static const unsigned char ${RESOURCE_VAR}_data[] = {${HEX_ARRAY}};\n")

        # Extraire la taille du nom du répertoire
        string(REGEX MATCH "([0-9]+)x([0-9]+)" SIZE_MATCH ${SIZE_NAME})
        if(CMAKE_MATCH_1 AND CMAKE_MATCH_2)
            set(WIDTH ${CMAKE_MATCH_1})
            set(HEIGHT ${CMAKE_MATCH_2})
        else()
            # Taille par défaut si le format n'est pas reconnu
            set(WIDTH "16")
            set(HEIGHT "16")
        endif()

        # Ajouter à l'enregistrement
        set(RESOURCE_REGISTRY "${RESOURCE_REGISTRY}    icons[\"${ICON_NAME}\"].push_back({${RESOURCE_VAR}_data, sizeof(${RESOURCE_VAR}_data), ${WIDTH}, ${HEIGHT}});\n")
    endforeach()

    # Générer la fonction d'initialisation
    set(RESOURCE_CODE "${RESOURCE_CODE}\nstatic const std::unordered_map<std::string, std::vector<EmbeddedIconData>>& GetEmbeddedIcons() {\n")
    set(RESOURCE_CODE "${RESOURCE_CODE}    static std::unordered_map<std::string, std::vector<EmbeddedIconData>> icons;\n")
    set(RESOURCE_CODE "${RESOURCE_CODE}    static bool initialized = false;\n")
    set(RESOURCE_CODE "${RESOURCE_CODE}    if (!initialized) {\n")
    set(RESOURCE_CODE "${RESOURCE_CODE}${RESOURCE_REGISTRY}")
    set(RESOURCE_CODE "${RESOURCE_CODE}        initialized = true;\n")
    set(RESOURCE_CODE "${RESOURCE_CODE}    }\n")
    set(RESOURCE_CODE "${RESOURCE_CODE}    return icons;\n")
    set(RESOURCE_CODE "${RESOURCE_CODE}}\n")

    file(WRITE ${OUTPUT_FILE} "#ifndef EMBEDDED_RESOURCES_H\n#define EMBEDDED_RESOURCES_H\n\n${RESOURCE_CODE}\n#endif\n")
endfunction()