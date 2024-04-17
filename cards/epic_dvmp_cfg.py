import Config.Core as cepgen
from Config.PDG_cfi import PDG
from Config.timer_cfi import timer # enable timing framework
from Integrators.miser_cfi import miser as integrator
from math import pi


process = cepgen.Module('epic',
    date = '2017-07-18',
    description = 'Select specific GPD types',
    tasks = [
        cepgen.Module('DVMPGeneratorService',
            general_configuration = cepgen.Parameters(
                meson = 'pi0',
            ),
            kinematic_range = cepgen.Parameters(
                range_y = (0.05, 0.95),
                range_Q2 = (1., 10.),
                range_t = (-1., -1.e-4),
                range_phi = (0.05, 2. * pi - 0.05),
                range_phiS = (0., 2. * pi),
                range_xB = (1.e-6, 1.),
            ),
            experimental_conditions = cepgen.Parameters(
                lepton_energy = 20.,
                lepton_type = 'e-',
                lepton_helicity = -1,
                hadron_energy = '1.00001*Mp',
                hadron_type = 'p',
                hadron_polarisation = [0., 0., 0.]
            ),
            computation_configuration = cepgen.Parameters(
                DVMPProcessModule = cepgen.Module('DVMPProcessGK06',
                    DVMPScalesModule = cepgen.Module('DVMPScalesQ2Multiplier',
                        Lambda = 1.,
                    ),
                    DVMPXiConverterModule = cepgen.Module('DVMPXiConverterXBToXi'),
                    DVMPConvolCoeffFunctionModule = cepgen.Module('DVMPCFFTables',
                        qcd_order_type = 'LO',
                    ),
                ),
            ),
            kinematic_configuration = cepgen.Parameters(
                DVMPKinematicModule = cepgen.Module('DVMPKinematicDefault'),
            ),
            rc_configuration = cepgen.Parameters(
                DVMPRCModule = cepgen.Module('DVMPRCNull'),
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
