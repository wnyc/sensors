function parse_nibble(a, b, c, d) {
    a = Number(Boolean(Number(a)));
    b = Number(Boolean(Number(b)));
    c = Number(Boolean(Number(c)));
    d = Number(Boolean(Number(d)));
    return a + b*2 + c*4 + d*8;
}

function parse_reversible_9bit_sequence(a, b, c, d, e, i, h, g, f) {
    e = Boolean(Number(e));
    low = parse_nibble(a, b, c, d);
    high = parse_nibble(f, g, h, i);
    if (e  && (low == high)) {
	return 256 + low;
    }
    if (e != (low > high)) {
	[low, high] = [high, low];
    }
    return low + high * 16;
}
      
function decode_temperature_to_celsius(value)
{
  return (Number(value) / 4.0) - 20.0;
} 


// (%i1) C2F(x) := x *9 / 5 + 32;
//                                         x 9
// (%o1)                         C2F(x) := --- + 32
//                                          5
// (%i2) V2C(x) := x / 4 - 20;
//                                          x
// (%o2)                          V2C(x) := - - 20
//                                          4
// fullratsimp(C2F(V2C(x)));
//                                    9 x - 80
// (%o3)                              ---------
//                                       20

function decode_temperature_to_fahrenheit(value) {
    //    return decode_temperature_to_celsius(value) * 1.8 + 32;
    return (value * 9 - 80 ) / 20 
}

