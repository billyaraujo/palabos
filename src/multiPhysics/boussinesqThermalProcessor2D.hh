/* This file is part of the Palabos library.
 *
 * Copyright (C) 2011-2015 FlowKit Sarl
 * Route d'Oron 2
 * 1010 Lausanne, Switzerland
 * E-mail contact: contact@flowkit.com
 *
 * The most recent release of Palabos can be downloaded at 
 * <http://www.palabos.org/>
 *
 * The library Palabos is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * The library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/* Main author: Orestis Malaspinas
 */

#ifndef BOUSSINESQ_THERMAL_PROCESSOR_2D_HH
#define BOUSSINESQ_THERMAL_PROCESSOR_2D_HH

#include "multiPhysics/boussinesqThermalProcessor2D.h"
#include "atomicBlock/blockLattice2D.h"
#include "core/util.h"
#include "finiteDifference/finiteDifference2D.h"
#include "latticeBoltzmann/geometricOperationTemplates.h"

namespace plb {

// ================= BoussinesqThermalProcessor2D ================= //
    
template< typename T,
          template<typename U1> class FluidDescriptor, 
          template<typename U2> class TemperatureDescriptor
        >
BoussinesqThermalProcessor2D<T,FluidDescriptor,TemperatureDescriptor>::
        BoussinesqThermalProcessor2D(T gravity_, T T0_, T deltaTemp_, Array<T,FluidDescriptor<T>::d> dir_)
    :  gravity(gravity_), T0(T0_), deltaTemp(deltaTemp_),
       dir(dir_)
{
    // We normalize the direction of the force vector.
    T normDir = std::sqrt(VectorTemplate<T,FluidDescriptor>::normSqr(dir));
    for (pluint iD = 0; iD < FluidDescriptor<T>::d; ++iD) {
        dir[iD] /= normDir;
    }
}

template< typename T,
          template<typename U1> class FluidDescriptor, 
          template<typename U2> class TemperatureDescriptor
        >
void BoussinesqThermalProcessor2D<T,FluidDescriptor,TemperatureDescriptor>::process (
        Box2D domain,
        BlockLattice2D<T,FluidDescriptor>& fluid,
        BlockLattice2D<T,TemperatureDescriptor>& temperature )
{
    typedef FluidDescriptor<T> D;
    enum {
        velOffset   = TemperatureDescriptor<T>::ExternalField::velocityBeginsAt,
        forceOffset = FluidDescriptor<T>::ExternalField::forceBeginsAt
    };
    Dot2D offset = computeRelativeDisplacement(fluid, temperature);
    
    Array<T,D::d> gravOverDeltaTemp (
            gravity*dir[0]/deltaTemp,
            gravity*dir[1]/deltaTemp );
           
    for (plint iX=domain.x0; iX<=domain.x1; ++iX) 
    {
        for (plint iY=domain.y0; iY<=domain.y1; ++iY) 
        {
            // Velocity coupling
            T *u = temperature.get(iX+offset.x,iY+offset.y).getExternal(velOffset);
            Array<T,FluidDescriptor<T>::d> vel;
            fluid.get(iX,iY).computeVelocity(vel);
            vel.to_cArray(u);
            
            // Computation of the Boussinesq force
            T *force = fluid.get(iX,iY).getExternal(forceOffset);
            // Temperature is the order-0 moment of the advection-diffusion lattice.
            //   You can compute it with the method computeDensity().
            T localTemperature = temperature.get(iX+offset.x,iY+offset.y).computeDensity();
            const T diffT = localTemperature - T0;
            for (pluint iD = 0; iD < D::d; ++iD) 
            {
                force[iD] = gravOverDeltaTemp[iD] * diffT;
            }
        }
    }
}

template< typename T,
          template<typename U1> class FluidDescriptor, 
          template<typename U2> class TemperatureDescriptor
        >
BoussinesqThermalProcessor2D<T,FluidDescriptor,TemperatureDescriptor>*
    BoussinesqThermalProcessor2D<T,FluidDescriptor,TemperatureDescriptor>::clone() const
{
    return new BoussinesqThermalProcessor2D<T,FluidDescriptor,TemperatureDescriptor>(*this);
}

// ================= CompleteBoussinesqThermalProcessor2D ================= //

template< typename T,
          template<typename U1> class FluidDescriptor, 
          template<typename U2> class TemperatureDescriptor
        >
CompleteBoussinesqThermalProcessor2D<T,FluidDescriptor,TemperatureDescriptor>::
        CompleteBoussinesqThermalProcessor2D(T gravity_, T T0_, T deltaTemp_, Array<T,FluidDescriptor<T>::d> dir_)
    :  gravity(gravity_), T0(T0_), deltaTemp(deltaTemp_),
       dir(dir_)
{
    // We normalize the direction of the force vector.
    T normDir = std::sqrt(VectorTemplate<T,FluidDescriptor>::normSqr(dir));
    for (pluint iD = 0; iD < FluidDescriptor<T>::d; ++iD) {
        dir[iD] /= normDir;
    }
}

template< typename T,
          template<typename U1> class FluidDescriptor, 
          template<typename U2> class TemperatureDescriptor
        >
void CompleteBoussinesqThermalProcessor2D<T,FluidDescriptor,TemperatureDescriptor>::process (
        Box2D domain,
        BlockLattice2D<T,FluidDescriptor>& fluid,
        BlockLattice2D<T,TemperatureDescriptor>& temperature )
{
    typedef FluidDescriptor<T> D;
    enum {
        rhoBarOffset   = TemperatureDescriptor<T>::ExternalField::rhoBarBeginsAt,
        jOffset   = TemperatureDescriptor<T>::ExternalField::velocityBeginsAt,
        piNeqOffset   = TemperatureDescriptor<T>::ExternalField::piNeqBeginsAt,
        omegaOffset   = TemperatureDescriptor<T>::ExternalField::omegaBeginsAt,
        forceOffset = FluidDescriptor<T>::ExternalField::forceBeginsAt
    };
    Dot2D offset = computeRelativeDisplacement(fluid, temperature);
    
    Array<T,D::d> gravOverDeltaTemp (
            gravity*dir[0]/deltaTemp,
            gravity*dir[1]/deltaTemp );
           
    for (plint iX=domain.x0; iX<=domain.x1; ++iX)  {
        for (plint iY=domain.y0; iY<=domain.y1; ++iY) {
            // Computation of the Boussinesq force
            T *force = fluid.get(iX,iY).getExternal(forceOffset);
            // Temperature is the order-0 moment of the advection-diffusion lattice.
            //   You can compute it with the method computeDensity().
            T localTemperature = temperature.get(iX+offset.x,iY+offset.y).computeDensity();
            const T diffT = localTemperature - T0;
            for (pluint iD = 0; iD < D::d; ++iD) {
                force[iD] = gravOverDeltaTemp[iD] * diffT;
            }

            // Velocity coupling
            T rhoBar;
            Array<T,FluidDescriptor<T>::d> j;
            Array<T,SymmetricTensor<T,FluidDescriptor>::n> piNeq;
            
            fluid.get(iX,iY).getDynamics().computeRhoBarJPiNeq(fluid.get(iX,iY),rhoBar,j,piNeq);
            for (plint iD = 0; iD < D::d; ++iD) {
                j[iD] += D::fullRho(rhoBar)*force[iD]*(T)0.5;
            }
            
            *temperature.get(iX+offset.x,iY+offset.y).getExternal(omegaOffset) = fluid.get(iX,iY).getDynamics().getOmega();
            *temperature.get(iX+offset.x,iY+offset.y).getExternal(rhoBarOffset) = rhoBar;
            j.to_cArray(temperature.get(iX+offset.x,iY+offset.y).getExternal(jOffset));
            piNeq.to_cArray(temperature.get(iX+offset.x,iY+offset.y).getExternal(piNeqOffset));

        }
    }
}

template< typename T,
          template<typename U1> class FluidDescriptor, 
          template<typename U2> class TemperatureDescriptor
        >
CompleteBoussinesqThermalProcessor2D<T,FluidDescriptor,TemperatureDescriptor>*
    CompleteBoussinesqThermalProcessor2D<T,FluidDescriptor,TemperatureDescriptor>::clone() const
{
    return new CompleteBoussinesqThermalProcessor2D<T,FluidDescriptor,TemperatureDescriptor>(*this);
}

}  // namespace plb

#endif  // BOUSSINESQ_THERMAL_PROCESSOR_2D_HH

