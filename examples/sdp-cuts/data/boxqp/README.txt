Problem is

  max  0.5*x'*Q*x + c'*x
  s.t. 0 <= x <= e

File format is

  n
  c'
  Q

"Basic" instances come from

    @Article{VanNem05b,
      author        = {D. Vandenbussche and G. Nemhauser},
      title         = {A branch-and-cut algorithm for nonconvex quadratic
                      programs with box constraints},
      journal       = {Mathematical Programming},
      volume        = {102},
      number        = {3},
      year          = {2005},
      pages         = {559--575}
    }

"Extended" instances come from

    @TechReport{BurVan06,
      author        = {S. Burer and D. Vandenbussche},
      title         = {Globally Solving Box-Constrained Nonconvex Quadratic
                       Programs with Semdefinite-Based Finite Branch-and-Bound},
      type          = {Manuscript},
      institution   = {Department of Management Sciences, University of Iowa},
      address       = {Iowa City, IA, USA},
      month         = {November},
      year          = {2006},
      note          = {Revised June 2007. To appear in {\em Computational
                       Optimization and Applications\/}.}
    }

"Extended2" instances come from

    @TechReport{Burer08,
      author        = {S. Burer},
      title         = {Optimizing a Polyhedral-Semidefinite Relaxation of Completely
                       Positive Programs},
      type          = {Manuscript},
      institution   = {Department of Management Sciences, University of Iowa},
      address       = {Iowa City, IA, USA},
      month         = {December},
      year          = {2008}
    }

Optimal values are as follows:

<basic>
spar020-100-1  7.06500000e+02
spar020-100-2  8.56500000e+02
spar020-100-3  7.72000000e+02
spar030-060-1  7.06000000e+02
spar030-060-2  1.37717308e+03
spar030-060-3  1.29350000e+03
spar030-070-1  6.54000000e+02
spar030-070-2  1.31300000e+03
spar030-070-3  1.65740147e+03
spar030-080-1  9.52728571e+02
spar030-080-2  1.59700000e+03
spar030-080-3  1.80978205e+03
spar030-090-1  1.29650000e+03
spar030-090-2  1.46684444e+03
spar030-090-3  1.49400000e+03
spar030-100-1  1.22712500e+03
spar030-100-2  1.26050000e+03
spar030-100-3  1.51105000e+03
spar040-030-1  8.39500000e+02
spar040-030-2  1.42900000e+03
spar040-030-3  1.08600000e+03
spar040-040-1  8.37000000e+02
spar040-040-2  1.42800000e+03
spar040-040-3  1.17350000e+03
spar040-050-1  1.15450000e+03
spar040-050-2  1.43098000e+03
spar040-050-3  1.65362857e+03
spar040-060-1  1.32266667e+03
spar040-060-2  2.00422857e+03
spar040-060-3  2.45450000e+03
spar040-070-1  1.60500000e+03
spar040-070-2  1.86750000e+03
spar040-070-3  2.43650000e+03
spar040-080-1  1.83850000e+03
spar040-080-2  1.95250000e+03
spar040-080-3  2.54550000e+03
spar040-090-1  2.13550000e+03
spar040-090-2  2.11300000e+03
spar040-090-3  2.53500000e+03
spar040-100-1  2.47637778e+03
spar040-100-2  2.10250000e+03
spar040-100-3  1.86607447e+03
spar050-030-1  1.32450000e+03
spar050-030-2  1.66800000e+03
spar050-030-3  1.45361250e+03
spar050-040-1  1.41100000e+03
spar050-040-2  1.74576190e+03
spar050-040-3  2.09450000e+03
spar050-050-1  1.19840909e+03
spar050-050-2  1.77600000e+03
spar050-050-3  2.10609783e+03
spar060-020-1  1.21200000e+03
spar060-020-2  1.92550000e+03
spar060-020-3  1.48300000e+03
<extended>
spar070-025-1  2.53890909e+03
spar070-025-2  1.88800000e+03
spar070-025-3  2.81228205e+03
spar070-050-1  3.25250000e+03
spar070-050-2  3.29600000e+03
spar070-050-3  4.30650000e+03
spar070-075-1  4.65550000e+03
spar070-075-2  3.86515385e+03
spar070-075-3  4.32940000e+03
spar080-025-1  3.15700000e+03
spar080-025-2  2.31234375e+03
spar080-025-3  3.09087500e+03
spar080-050-1  3.44810000e+03
spar080-050-2  4.44920455e+03
spar080-050-3  4.88600000e+03
spar080-075-1  5.89600000e+03
spar080-075-2  5.34099999e+03
spar080-075-3  5.98050000e+03
spar090-025-1  3.37250000e+03
spar090-025-2  3.50028576e+03
spar090-025-3  4.29900000e+03
spar090-050-1  5.15200000e+03
spar090-050-2  5.38650000e+03
spar090-050-3  6.15100000e+03
spar090-075-1  6.26745000e+03
spar090-075-2  5.64750000e+03
spar090-075-3  6.45000000e+03
spar100-025-1  4.02750000e+03
spar100-025-2  3.89255556e+03
spar100-025-3  4.45350000e+03
spar100-050-1  5.49000000e+03
spar100-050-2  5.86600000e+03
spar100-050-3  6.48500000e+03
spar100-075-1  7.38419565e+03
spar100-075-2  6.75550000e+03
spar100-075-3  7.55400000e+03
<extended2>
spar125-025-1  5.57200000e+03
spar125-025-2  6.15606061e+03
spar125-025-3  6.81550000e+03
spar125-050-1  9.30838194e+03
spar125-050-2  8.39500000e+03
spar125-050-3  8.34391429e+03
spar125-075-1  1.23300000e+04
spar125-075-2  1.03824694e+04
spar125-075-3  9.63550000e+03
