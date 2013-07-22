#include "cinder/app/App.h"
#include "cinder/ImageIo.h"

#include "Utils.h"

using namespace ci;
using namespace std;

namespace mndl {

vector< pair< string, gl::Texture > > loadTextures( const fs::path &relativeDir )
{
    vector< pair< string, gl::Texture > > textures;

    fs::path dataPath = app::getAssetPath( relativeDir );

	if ( dataPath.empty() )
	{
		app::console() << "Could not find texture directory assets/" << relativeDir.string() << std::endl;
	}
	else
	{
		for ( fs::directory_iterator it( dataPath ); it != fs::directory_iterator(); ++it )
		{
			if ( fs::is_regular_file( *it ) )
			{
				try
				{
					gl::Texture t = loadImage( app::loadAsset( relativeDir / it->path().filename() ) );
					textures.push_back( pair< string, gl::Texture >( it->path().filename().string(), t ) );
				}
				catch ( const ImageIoException &exc  )
				{
					app::console() << "Unable to load image " << it->path() << ": " << exc.what() << std::endl;
				}
			}
		}
	}

    return textures;
}

} // namespace mndl
