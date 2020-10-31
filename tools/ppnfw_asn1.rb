#!/usr/bin/ruby

require 'openssl'

if ARGV.length < 2
    print File.basename(__FILE__) + " <outfile> ppn-fw=<fw file> [ ppn-fw-args=<fwa file> ]\n"
    exit 1
end

outFn = ARGV[0]
inPairs = ARGV[1..-1]

blob = OpenSSL::ASN1::Sequence([], 1, :IMPLICIT, :APPLICATION)
inPairs.each do |pair|
    key, val = pair.split('=', 2)

    blob.value << OpenSSL::ASN1::UTF8String(key)
    blob.value << OpenSSL::ASN1::OctetString(File.read(val))
end

File.open(outFn, 'w') {|f| f.write(blob.to_der) }
