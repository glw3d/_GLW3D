/**
Author: Mario J. Martin <dominonurbs$gmail.com>

*******************************************************************************/

#ifndef _HGW_CAMERA_HPP
#define _HGW_CAMERA_HPP

#include <math.h>

#include "signatures.h"
#include "defines.h"
#include "gwdata.h"
#include "gwmath.h"
#include "gwcamera.h"
#include "signatures.h"
#include "gwcolors.h"
#include "Stream.hpp"

#include "ShaderGeneric.hpp"
#include "Mesh.hpp"

namespace gw
{
    struct Camera : public gwCamera
    {
        /* Signature for safer upcasting */
        int signature;

        /* List of meshes to be rendered */
        DynamicStack< Mesh > render_list;

        gwMatrix4f view_matrix;

        Camera()
        {
            signature = _GW_SIGNATURE_CAMERA;
            type = GW_CAMERA_VIEW_CAD;
            view_matrix = gwMatrix4f_identity();

            active = 1;

            eye.x = 0.0f;
            eye.y = 0.0f;
            eye.z = 0.0f;

            rotation_angles.x = 0.0f;
            rotation_angles.y = 0.0f;
            rotation_angles.z = 0.0f;

            scale.x = 1.0f;
            scale.y = 1.0f;
            scale.z = 1.0f;

            zoom = 1.0f;
            farClip = 10.0f;
            nearClip = 0.001f;
            field_angle = 45;

            target.x = 0;
            target.y = 0;
            target.z = 0;

            lookup.x = 0;
            lookup.y = 0;
            lookup.z = 1;

            angle_rounding = 5;

            boundary_frame = nullptr;

            on_mouse_click = nullptr;
            on_mouse_move = nullptr;
            on_mouse_release = nullptr;
            on_mouse_scroll = nullptr;
            on_keyboard = nullptr;
        }

        /* Rounding a floating point value to the nearest integer */
        template< typename T >
        int cintf( T x )
        {
            double n;
            double d = modf( (double)x, &n );

            if (x >= 0){
                if (d >= 0.5){
                    return (int)ceil( x );
                }
                else{
                    return (int)floor( x );
                }
            }
            else{
                if (-d >= 0.5){
                    return (int)floor( x );
                }
                else{
                    return (int)ceil( x );
                }
            }
        }

        void round_angles()
        {
            if (angle_rounding > 0){
                gwFloat ax = rotation_angles.x / angle_rounding;
                rotation_angles.x
                    = (gwFloat)(cintf< gwFloat >( ax ) * angle_rounding);

                gwFloat ay = rotation_angles.y / angle_rounding;
                rotation_angles.y
                    = (gwFloat)(cintf< gwFloat >( ay ) * angle_rounding);

                gwFloat az = rotation_angles.z / angle_rounding;
                rotation_angles.z
                    = (gwFloat)(cintf< gwFloat >( az ) * angle_rounding);
            }
        }

        void render( const int viewport_width, const int viewport_height
            , const ShaderGeneric* shader, const int gl_version )
        {
            /* Draw the scene */
            DynamicStack<Mesh>* token_mesh = &render_list;
            while (token_mesh != nullptr){
                Mesh* mesh = token_mesh->obj;
                token_mesh = token_mesh->next;
                if (mesh != nullptr){
                    mesh->render( mesh, shader, this->view_matrix
                        , viewport_width, viewport_height, gl_version );
                }
            }
        }

        /* Update the transformation matrix in all meshes associated to the camera */
        void update_transformations( const int viewport_width, const int viewport_height )
        {
            gwVector3f parent_blit = { 0, 0, 0 };

			/* Update the local matrix */
			DynamicStack< Mesh >* token_mesh = &render_list;
			while ( token_mesh != nullptr ){
				Mesh* mesh = token_mesh->obj;
				token_mesh = token_mesh->next;
				if ( mesh != nullptr ){
					mesh->update_local_matrix();
				}
			}

			/* Update the transformation matrix */
			token_mesh = &render_list;
			while ( token_mesh != nullptr ){
				Mesh* mesh = token_mesh->obj;
				token_mesh = token_mesh->next;
				if ( mesh != nullptr ){
					mesh->update_transformation_matrix();
				}
			}
		}

		/* Get the box of a mesh, including its siblings a childs */
		static void get_mesh_box_iterate( const gwMesh* mesh, gwVector3f* pmax, gwVector3f* pmin, void* vertex_check[128] )
		{
			/* Check that the vertex stream is not already used */
			size_t iv = 0;
			for ( ; iv < 127; iv++ ){
				if ( vertex_check[iv] == nullptr ){
					break;
				}
				if ( vertex_check[iv] == mesh->vertex ){
					return;
				}
			}
			gwVector3f max, min;
			if ( mesh->vertex != nullptr && mesh->vertex->length > 0 ){
				vertex_check[iv] = mesh->vertex;

				max = mesh->vertex->stream[0].position;
				min = mesh->vertex->stream[0].position;

				for ( size_t i = 0; i < mesh->vertex->length; i++ ){
					if ( mesh->vertex->stream[i].position.x > max.x ){
						max.x = mesh->vertex->stream[i].position.x;
					}
					if ( mesh->vertex->stream[i].position.y > max.y ){
						max.y = mesh->vertex->stream[i].position.y;
					}
					if ( mesh->vertex->stream[i].position.z > max.z ){
						max.z = mesh->vertex->stream[i].position.z;
					}

					if ( mesh->vertex->stream[i].position.x < min.x ){
						min.x = mesh->vertex->stream[i].position.x;
					}
					if ( mesh->vertex->stream[i].position.y < min.y ){
						min.y = mesh->vertex->stream[i].position.y;
					}
					if ( mesh->vertex->stream[i].position.z < min.z ){
						min.z = mesh->vertex->stream[i].position.z;
					}
				}

				/* ¡¡¡MATRIX TRANSFORMATION!!! */

				/* Check if the values are assigned for the first time */
				if ( pmax->x == 0 && pmax->y == 0 && pmax->z == 0
					&& pmin->x == 0 && pmin->y == 0 && pmin->z == 0 )
				{
					pmax->x = max.x;
					pmax->y = max.y;
					pmax->z = max.z;

					pmin->x = min.x;
					pmin->y = min.y;
					pmin->z = min.z;
				}

				if ( pmax->x < max.x ) pmax->x = max.x;
				if ( pmax->y < max.y ) pmax->y = max.y;
				if ( pmax->z < max.z ) pmax->z = max.z;

				if ( pmin->x > min.x ) pmin->x = min.x;
				if ( pmin->y > min.y ) pmin->y = min.y;
				if ( pmin->z > min.z ) pmin->z = min.z;
			}
		}

		/* Calculate the mesh box */
		static void get_mesh_box( const gwMesh* mesh, gwVector3f* pmax, gwVector3f* pmin )
		{
			void* vertex_check[128] = { nullptr };
			get_mesh_box_iterate( mesh, pmax, pmin, vertex_check );
		}

		/* Sets the view to see all the scene */
		void autofit()
		{
			if ( this->type == GW_CAMERA_VIEW_CAD ){
				gwVector3f max = { 0, 0, 0 };
				gwVector3f min = { 0, 0, 0 };
				int index = 0;
				
				DynamicStack< Mesh >* token_mesh = &render_list;
				while ( token_mesh != nullptr ){

					Mesh* mesh = token_mesh->obj;
					index++;
					if ( mesh != nullptr ){
						get_mesh_box( mesh, &max, &min );
					}
					else{
						break;
					}
					token_mesh = token_mesh->next;
				}

				if ( max.x - min.x > 10 * ( max.y - min.y ) && max.z - min.z > 10 * ( max.y - min.y ) ){
					this->rotation_angles.x = 0;
					this->rotation_angles.y = 180;
					this->rotation_angles.z = 90;
				}
				else if ( max.y - min.y > 10 * ( max.x - min.x ) && max.z - min.z > 10 * ( max.x - min.x ) ){
					this->rotation_angles.x = 0;
					this->rotation_angles.y = 90;
					this->rotation_angles.z = 0;
				}
				else if ( max.x - min.x > 10 * ( max.z - min.z ) && max.y - min.y > 10 * ( max.z - min.z ) ){
					this->rotation_angles.x = 90;
					this->rotation_angles.y = 0;
					this->rotation_angles.z = 0;
				}
				else
				{
					/* Default for 3D views */
					this->rotation_angles.x = 0;
					this->rotation_angles.y = 180;
					this->rotation_angles.z = 90;
				}

				this->eye.x = -( max.x + min.x ) / 2;
				this->eye.y = -( max.y + min.y ) / 2;
				this->eye.z = -( max.z + min.z ) / 2;

				float zoom = 1;

				if ( max.x != min.x ){
					zoom = max.x - min.x;
				}
				if ( max.y != min.y && zoom < max.y - min.y ){
					zoom = max.y - min.y;
				}
				if ( max.z != min.z && zoom < max.z - min.z ){
					zoom = max.z - min.z;
				}
				this->zoom = 1.5f / zoom;
			}
		}
    };
}

#endif _HGW_CAMERA_HPP
/**/
