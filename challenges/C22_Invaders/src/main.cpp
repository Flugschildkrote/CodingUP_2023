#include <cstdlib>
#include <iostream>
#include <anubis/Anubis.hpp>
#include <anubis/Tree.hpp>
#include <anubis/net/Http.hpp>
#include <cassert>
#include <stb_image.h>
#include <stb_image_write.h>
#include <fstream>
#include <anubis/util/Iterator.hpp>
#include <anubis/util/MemoryView.hpp>
#include <anubis/image/Image.hpp>
#include <anubis/image/ImageView.hpp>
#include <anubis/coding_up/Net.hpp>
#include <anubis/util/Base64.hpp>

struct RGB_Pixel {
    uint8_t r;
    uint8_t g;
    uint8_t b;

    bool any(void) const noexcept { return bool(r | g | b); }
    bool operator&(const RGB_Pixel& other) { return any() && other.any(); }
};

using Image = anubis::Image;

template <typename T>
using ImageView = anubis::ImageView<T>;

struct InvadersInfo {
    uint32_t tileWidth;
    uint32_t tileHeight;
    Image invadersImage;
    std::vector<ImageView<RGB_Pixel>> invaders;
};

/**
 * Lis le fichier de space invaders de référence. 
 * Sur cette image un pixel (d'un invader) n'est pas égal à un pixel de l'image
 * Ici c'est calculé mais en vrai on pourrait juste regarder sur l'image et trouver que un pixel 
 * d'un invader représente 10x10 (vrais) pixels sur l'image
 */
InvadersInfo readSpaceInvadersReferenceFile(void) {

    InvadersInfo invadersInfo;
    invadersInfo.invadersImage = Image("data/invaders_ref.png", 3);
    Image& refInvaders = invadersInfo.invadersImage;

    const uint32_t w = refInvaders.getWidth();
    const uint32_t h = refInvaders.getHeight();
    const uint8_t* pixels = refInvaders.data();
    int pixelScale = w;
    int pixelCount = 0;
    int tileHeight = 0;

    // search for pixel scale
    for (uint32_t y = 0; y < h; ++y)
    {
        for (uint32_t x = 0; x < w; ++x)
        {
            uint32_t pixelPos = (y * w + x)*3;
            uint32_t pixelTotal = pixels[pixelPos] + pixels[pixelPos + 1] + pixels[pixelPos + 2];

            if (pixelTotal > 0 && pixelTotal < (255 * 3)) {
                tileHeight = y;
                goto end_scan;
            }

            if (pixelTotal > 0) {
                pixelCount++;
            }
            else if(pixelCount > 0){

                pixelScale = std::min<int>(pixelCount, pixelScale);
                pixelCount = 0;
            }
        }
    }
end_scan:


    int numAliens = 9;
    tileHeight /= pixelScale;
    int tileWidth = w / pixelScale / numAliens;

    // Une ImageView est une vue d'une portion d'une image. Par exemple si on est intéressé par une portion de 10x10 pixels d'une image de 100x100 pixels
    // une image view permet de simplifier les traitements au lieu de le faire à la main. 

    std::vector<ImageView<RGB_Pixel>> &alienReferences = invadersInfo.invaders;
    for (uint32_t i = 0; i < numAliens; ++i) {
        alienReferences.push_back(refInvaders.getView<RGB_Pixel>(i* tileWidth, 0, tileWidth, tileHeight, pixelScale));
    }
    invadersInfo.tileWidth = tileWidth;
    invadersInfo.tileHeight = tileHeight;

    std::cout << "Pixel size=" << pixelScale << ", " << tileWidth << "x" << tileHeight << std::endl;
    return invadersInfo;
}

int main(void)
{
    try {
        // Scan de l'image de référence (C22_Invaders/data/invaders_ref.png)
        InvadersInfo invadersInfo = readSpaceInvadersReferenceFile();
 
        // Contexte réseau pour CodingUp
        anubis::coding_up::CodingUpNet net("pydefis.callicode.fr", "/defis/C22_Invaders01/get/Anubis29/57f04", "/defis/C22_Invaders01/post/Anubis29/57f04");

        // Récupération de l'image depuis l'url (encodée en base64)
        std::string base64Data = net.Get();
        std::cout << "base64 data length=" << base64Data.length() << std::endl;

        // Décodage de la chaine en base64 -> byte[]
        std::vector<uint8_t> pixels = anubis::util::base64Decode(base64Data);

        // Construction de l'image à partir des données décodées
        Image mysteryPicture(pixels.data(), pixels.size(), 3);

        // Une image est composée de 6x4 invaders, on itère pour chaque invader
        std::string invadersString = "";
        for (int mysteryInvaderId = 0; mysteryInvaderId < (6 * 4); ++mysteryInvaderId) 
        {
            // L'image fais 6 invaders de large. On calcule la pos (x, y) d'un invader ) partir de son index 
            size_t x = (mysteryInvaderId % 6) * invadersInfo.tileWidth;
            size_t y = (mysteryInvaderId / 6) * invadersInfo.tileHeight;

            // Création d'une vue sur cet invader (le 4 c'est car après analyse de l'image recue, un pixel d'un invader est de taille 4x4 pixels)
            ImageView<RGB_Pixel> mysteryInvader = mysteryPicture.getView<RGB_Pixel>(x, y, invadersInfo.tileWidth, invadersInfo.tileHeight, 4);

            // Comparaison de l'invader mystère avec chaque invader de référence
            // Si on a un match, alors on ajoute la lettre à la chaine de caractères 
            for (int refInvaderId = 0; refInvaderId < invadersInfo.invaders.size(); ++refInvaderId)
            {
                ImageView<RGB_Pixel>& refInvader = invadersInfo.invaders[refInvaderId];
                bool match = true;
                // Pour chaque ligne
                for (size_t y = 0; y < refInvader.size(); ++y)
                {
                    auto refRow = refInvader[y];
                    auto firstInvaderRow = mysteryInvader[y];

                    // Pour chaque colonne
                    for (size_t x = 0; x < refRow.size() && match; ++x)
                    {
                        // Comparaison des pixels de la vue (osef de la couleur, comparaison noir/blanc)
                        match = refRow[x].any() == firstInvaderRow[x].any();
                    }
                }

                // Si match, on ajoute la lettre corespondante
                if (match) {
                    invadersString += char('A' + refInvaderId);
                }
            }
        }

        // Post du résultat
        std::cout << net.Post(invadersString) << std::endl;

    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    return EXIT_SUCCESS;
}