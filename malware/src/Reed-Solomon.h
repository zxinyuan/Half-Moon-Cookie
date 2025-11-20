#ifndef KRFingerprint_H
#define KRFingerprint_H
#include "NTL/ZZ.h"
#include "constants.h"
#include "NTL/ZZ_pXFactoring.h"
#include "NTL/vec_ZZ_p.h"
#include "NTL/ZZ.h"
#include "emp-tool/emp-tool.h"


using namespace NTL;
using namespace std;


vec_ZZ_p encode_rs(vec_ZZ_p x, int inputLength, vec_ZZ_p Alpha, vec_ZZ_p Beta)
{
        //int l = n - p;
        mat_ZZ_p G;
        mat_ZZ_p H;
        int L[l];


        // Defining the value of u
        vec_ZZ_p u = random_vec_ZZ_p(k);
        for(int i = 0; i < inputLength; i++)
        {
            u[i] = x[i];
            if(u[i] == 0){
                cout << "ERROR in encoding" << endl;
                exit(0);
            }
        }    

        // Defining indices to add noise
        for(int i = 0; i < l; i++){
            L[i] = i;   // this is not a random selection!
        }


        
        // Defining the polynomial P    
        ZZ_pX polyP = interpolate(Alpha, u);
        //cout << "interpolation done" << endl;
        // Defining the Generator matrix
       
       /*
        G.SetDims(n, k);

        for(int i = 0; i < n; i++)
        {
            for(int j = 0; j < k; j++)
            {
                G[i][j] = (power(Beta[i],j) * coeff(polyP, j)) * inv(u[j]);
            }
        }
        */
        // sanity check
        /*
        for(int i = 0; i < n; i++)
        {
            if((G*u)[i] != eval(polyP, Beta[i]))
            {
            cout << "Error in computing G:" << (G*u)[i] << " " << eval(polyP, Beta[i]) << endl;
                break;
            }
        }
        */

        // generate v
        vec_ZZ_p v = random_vec_ZZ_p(n);
        for(int i = 0; i < l; ++i)
        {
            v[i] = eval(polyP, Beta[i]); //(G*u)[i]; // This is a hack for fast encoding generation
        }

        // Defining the polynomial Q   
        vec_ZZ_p V;
        V.SetLength(l);
    
    for(int i = 0; i < l; ++i)
        {
            V[i] = v[i];
        }
    
        vec_ZZ_p Beta_sub;
        Beta_sub.SetLength(l);

        for(int i = 0; i < l; ++i)
        {
            Beta_sub[i] = Beta[i];
        }
        ZZ_pX polyQ = interpolate(Beta_sub, V);

        //TODO: need to fix this for the correct set of random points
        /*
        //generate H
        H.SetDims(k, 2*k - 1);
        for(int i = 0; i < k; i++)
        {
            for(int j = 0; j < 2*(k)-1; j++)
            {
                H[i][j] = (power(Alpha[i],j) * coeff(polyQ, j))/V[l-1];
            }
        }
        */


        return V;
    }


#endif