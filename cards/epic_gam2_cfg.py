import Config.Core as cepgen
from Config.PDG_cfi import PDG
from Config.timer_cfi import timer # enable timing framework
from Integrators.miser_cfi import miser as integrator
from math import pi


process = cepgen.Module('epic',
    date = '2017-07-18',
    description = 'Select specific GPD types',
    tasks = [
        cepgen.Module('GAM2GeneratorService',
            kinematic_range = cepgen.Parameters(
                range_y = (0.05, 0.95),
                range_Q2 = (0.15, 5.),
                range_t = (-1., -1.e-4),
                range_uPrim = (-10., 0.),
                range_Mgg2 = (1., 10.),
                range_phi = (0.05, 2. * pi - 0.05),
                range_xB = (1.e-6, 1.),
            ),
            experimental_conditions = cepgen.Parameters(
                lepton_energy = 10.,
                lepton_type = 'e-',
                lepton_helicity = -1,
                hadron_energy = '1.00001*Mp',
                hadron_type = 'p',
                hadron_polarisation = [0., 0., 0.]
            ),
            computation_configuration = cepgen.Parameters(
                GAM2ProcessModule = cepgen.Module('GAM2ProcessGPSSW21',
                    GAM2ScalesModule = cepgen.Module('GAM2ScalesMgg2Multiplier',
                        Lambda = 1.,
                    ),
                    GAM2XiConverterModule = cepgen.Module('GAM2XiConverterExact'),
                    GAM2ConvolCoeffFunctionModule = cepgen.Module('GAM2CFFStandard',
                        qcd_order_type = 'LO',
                        GPDModule = cepgen.Module('GPDGK16')
                    ),
                ),
            ),
            kinematic_configuration = cepgen.Parameters(
                GAM2KinematicModule = cepgen.Module('GAM2KinematicDefault'),
            ),
            rc_configuration = cepgen.Parameters(
                GAM2RCModule = cepgen.Module('GAM2RCNull'),
            ),
        ),
    ],
)

generator = cepgen.Parameters(
    numEvents = 100
)
text = cepgen.Module('text',
    histVariables={
        'e(5)': cepgen.Parameters(xbins=[float(bin) for bin in range(0, 250, 10)]),
    }
)
output = cepgen.Sequence(text)
