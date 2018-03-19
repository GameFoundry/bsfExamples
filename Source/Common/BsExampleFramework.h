#pragma once

#include "BsPrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Resources/BsResources.h"
#include "Mesh/BsMesh.h"
#include "Importer/BsImporter.h"
#include "Importer/BsMeshImportOptions.h"
#include "Importer/BsTextureImportOptions.h"

namespace bs
{
	/** Various helper functionality used throught the examples. */
	class ExampleFramework
	{
	public:
		/** 
		 * Imports a mesh at the provided path and saves it for later use. If the mesh was previously imported, it will
		 * instead just load the saved mesh.
		 * 
		 * Mesh can optionally be scaled on import by using the 'scale' parameter. */
		static HMesh loadMesh(const Path& path, float scale = 1.0f)
		{
			Path assetPath = path;
			assetPath.setExtension(path.getExtension() + ".asset");

			HMesh model = gResources().load<Mesh>(assetPath);
			if (model == nullptr) // Mesh file doesn't exist, import from the source file.
			{
				// When importing you may specify optional import options that control how is the asset imported.
				SPtr<ImportOptions> meshImportOptions = Importer::instance().createImportOptions(path);

				// rtti_is_of_type checks if the import options are of valid type, in case the provided path is pointing to a
				// non-mesh resource. This is similar to dynamic_cast but uses Banshee internal RTTI system for type checking.
				if (rtti_is_of_type<MeshImportOptions>(meshImportOptions))
				{
					MeshImportOptions* importOptions = static_cast<MeshImportOptions*>(meshImportOptions.get());

					importOptions->setImportScale(scale);
				}

				model = gImporter().import<Mesh>(path, meshImportOptions);

				// Save for later use, so we don't have to import on the next run.
				gResources().save(model, assetPath, true);
			}

			return model;
		}

		/**
		 * Imports a texture at the provided path and saves it for later use. If the texture was previously imported, it 
		 * will instead just load the saved texture.
		 * 
		 * Textures not in sRGB space (e.g. normal maps) need to be specially marked by setting 'isSRGB' to false. Also 
		 * allows for conversion of texture to cubemap by setting the 'isCubemap' parameter. If the data should be imported
		 * in a floating point format, specify 'isHDR' to true.
		 */
		static HTexture loadTexture(const Path& path, bool isSRGB = true, bool isCubemap = false, bool isHDR = false)
		{
			Path assetPath = path;
			assetPath.setExtension(path.getExtension() + ".asset");

			HTexture texture = gResources().load<Texture>(assetPath);
			if (texture == nullptr) // Texture file doesn't exist, import from the source file.
			{
				// When importing you may specify optional import options that control how is the asset imported.
				SPtr<ImportOptions> textureImportOptions = Importer::instance().createImportOptions(path);

				// rtti_is_of_type checks if the import options are of valid type, in case the provided path is pointing to a 
				// non-texture resource. This is similar to dynamic_cast but uses Banshee internal RTTI system for type checking.
				if (rtti_is_of_type<TextureImportOptions>(textureImportOptions))
				{
					TextureImportOptions* importOptions = static_cast<TextureImportOptions*>(textureImportOptions.get());

					// We want maximum number of mipmaps to be generated
					importOptions->setGenerateMipmaps(true);

					// If the texture is in sRGB space the system needs to know about it
					importOptions->setSRGB(isSRGB);

					// Ensures we can save the texture contents
					importOptions->setCPUCached(true);

					// Import as cubemap if needed
					importOptions->setIsCubemap(isCubemap);

					// If importing as cubemap, assume source is a panorama
					importOptions->setCubemapSourceType(CubemapSourceType::Cylindrical);

					// Importing using a HDR format if requested
					if (isHDR)
						importOptions->setFormat(PF_RG11B10F);
				}

				// Import texture with specified import options
				texture = gImporter().import<Texture>(path, textureImportOptions);

				// Save for later use, so we don't have to import on the next run.
				gResources().save(texture, assetPath, true);
			}

			return texture;
		}
	};
}
