#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_lib/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_lib/stb_image_write.h"


// Personal code
#include "image.hpp"

/* =========== PROGRAMING NOTES =============
 *
 * Important: images should only be manipulated under the form of
 *            an Image object.
============================================= */

int main( int argc, char* argv[] ){
	std::cout << "Hello World!" << std::endl;
	std::cout << "You have entered " << argc-1 << " arguments." << std::endl;
	// TODO: manage arguments
	for ( int i=1 ; i < argc ; ++i ){
		std::cout << *argv[i] << std::endl;
	}
	// Read an image file
	// std::string file_name = "lena.ppm"; // 512x512
	// ImagePPM* img = new ImagePPM( file_name );
        
        
        // Read a jpeg image file
        int jpg_width, jpg_height, jpg_bpp;
        std::string jpg_file = "test2_jpg.jpg";
        uint8_t* jpg_image = stbi_load( jpg_file.c_str(), &jpg_width, &jpg_height, &jpg_bpp, 0); // 0 is a default value
        std::cout << "The file " << jpg_file << " has been opened. It is " << jpg_width << " by " << jpg_height << " with " << jpg_bpp << " channels." << std::endl;
        // Copy data & create PPM file
        std::vector< unsigned char > copy_jpg;
        for( int i=0 ; i<3*jpg_width*jpg_height ; ++i ){
            copy_jpg.push_back( jpg_image[ i ]);
        }
        // Is the data in the vector valid ?
        // Is it on 8 bits ?
        ImagePPM* img = new ImagePPM( copy_jpg, jpg_width, jpg_height, 255 );
        // Free jpg and save PPM
        stbi_image_free( jpg_image );
        // jpg_ppm->writePPM( "jpg_copy.ppm" );
        // delete jpg_ppm;

        
        
        // Load reference images
        /*std::vector< ImagePPM > ref_img;
        ref_img.push_back( ImagePPM( "1.ppm" ) );
        ref_img.push_back( ImagePPM( "2.ppm" ) );
        ref_img.push_back( ImagePPM( "3.ppm" ) );
        ref_img.push_back( ImagePPM( "4.ppm" ) );
	
        // Modify it
        // /!!!!\ does not return, deletes memory
        std::vector< Image > blocks = img->splitImage( 64 );

        // Compare each bit with the references
        for( auto& block : blocks ){
            long distance = 1e12, new_distance = 9;
            int best_index = 0;
            for( int i=0 ; i<4 ; ++i ){ // Needs to be set to the size of ref_img (4)
                new_distance = ref_img[i].compareAbs( *block.getPixelValues() );
                // std::cout << new_distance << " ";
                if( new_distance < distance ){
                    best_index = i;
                    distance = new_distance;
                }
            }
            // std::cout << best_index << " ";
            block = ref_img[ best_index ];
        }*/
        
        // What's next?
        // => create some dummy basis in the form of 0-1 bitmaps
        // => allow to compare individual layers and not a full image
        // => new block = sum_of( RGB values * best shape )
        const int BASIS_SIZE = 4;
        std::vector< std::vector<unsigned char> > basis = 
                                              {{0, 1, 0, 0,
                                                0, 1, 0, 0,
                                                0, 1, 0, 0,
                                                0, 1, 0, 0 },
                                               {0, 0, 0, 0,
                                                0, 0, 0, 0,
                                                1, 1, 1, 1,
                                                0, 0, 0, 0 },
                                               {0, 0, 0, 1,
                                                0, 0, 1, 0,
                                                0, 1, 0, 0,
                                                1, 0, 0, 0 },
                                               {1, 0, 0, 0,
                                                0, 1, 0, 0,
                                                0, 0, 1, 0,
                                                0, 0, 0, 1 }};
        /*std::vector< std::vector<unsigned char> > basis = 
                                              {{0, 1, 0, 0,
                                                1, 1, 1, 1,
                                                0, 1, 0, 0,
                                                0, 1, 0, 0 },
                                              
                                               {0, 0, 1, 0,
                                                0, 0, 1, 0,
                                                1, 1, 1, 1,
                                                0, 0, 1, 0 },

                                               {1, 0, 0, 1,
                                                1, 0, 0, 1,
                                                1, 0, 0, 1,
                                                1, 0, 0, 1 },
                                              
                                               {1, 1, 1, 1,
                                                0, 0, 0, 0,
                                                0, 0, 0, 0,
                                                1, 1, 1, 1 }};*/


        std::vector< Image > new_blocks = img->splitImage( 640 ); // Careful here
        for( auto& block : new_blocks ){
            for( int i=0 ; i<3 ; ++i){
                std::array< int, BASIS_SIZE+1 > scores; // Stores the index of the best last
                scores.fill( 0 );
                // Fill the scores array
                for( int j=0 ; j<BASIS_SIZE ; ++j ){
                    // Compare layer with basis' elements and compute score
                    for( int k=0 ; k<BASIS_SIZE*BASIS_SIZE ; ++k ){
                        scores[ j ] += basis[j][k] * block.getPixelValues()->at(i)[k];
                    }
                    if ( scores[j] > scores[ scores[BASIS_SIZE] ] )
                        scores[ BASIS_SIZE ] = j;
                }
                //std::cout << "Final scores: " << scores[0] << "-" << scores[1] << "-" << scores[2] << "-" << scores[3] << std::endl;
                
                // Compute mean value for current layer
                int mean_value = 0;
                for( int j=0 ; j<BASIS_SIZE*BASIS_SIZE ; ++j ){
                    mean_value += block.getPixelValues()->at(i)[j];
                }
                mean_value = mean_value/BASIS_SIZE/BASIS_SIZE; 

                // Assign best basis' element, with mean value to block's layer
                for( int j=0 ; j<BASIS_SIZE*BASIS_SIZE ; ++j ){
                    block.getPixelValues()->at(i)[j] =
                        mean_value * basis[ scores[BASIS_SIZE] ][j];
                }
            }
        }

        // Build a new Image from blocks
        Image filtered( new_blocks, jpg_width, jpg_height, 255 );
        // Reconstruct a single array of interleaved pixels
        /*uint8_t* filtered_data[ 3*jpg_width*jpg_height ];
        for( int i=0 ; i<jpg_width*jpg_height ; ++i){
                            
        }*/
        std::vector< uint8_t > filtered_data;
        for( int i=0 ; i<jpg_width*jpg_height ; ++i ){
            filtered_data.push_back( filtered.getPixelValues()->at(0)[ i ] ); // Red
            filtered_data.push_back( filtered.getPixelValues()->at(1)[ i ] ); // Green
            filtered_data.push_back( filtered.getPixelValues()->at(2)[ i ] ); // Blue
        }

        // file, w, h, channels, pure data, quality 0-100
        stbi_write_jpg( "out.jpg", jpg_width, jpg_height, 3, filtered_data.data(), 100 );
                


        // ImagePPM* new_img = new ImagePPM( blocks, 512, 512, 255 );
        // b_img->addBrightness( 40 );

	// Write an image file
	// new_img->writePPM( "new_test.ppm" );
	// b_img->writePPM( "b_test.ppm" );
	delete img;
        // delete new_img;
        // delete b_img;
}
