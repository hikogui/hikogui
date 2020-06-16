Pod::Spec.new do |s|
    s.name         = "Rhea"
    s.version      = '0.2.4'
    s.summary      = "A modern c++ constraint solver based on Cassowary."
    s.homepage     = "https://github.com/Nocte-/rhea"
    s.authors      = { 
    	"Nocte" => "nocte@hippie.nu"
    	}
    s.license      = 'MIT'
    s.source       = { 
        :git => "https://github.com/Nocte-/rhea.git", 
        :tag => s.version.to_s
        }
    s.default_subspec = 'Default'
    s.library = 'c++'
    s.xcconfig = {
       'CLANG_CXX_LANGUAGE_STANDARD' => 'c++11',
       'CLANG_CXX_LIBRARY' => 'libc++'
    }
    s.platform      = :ios, '6.0'
    s.requires_arc  = true

    s.subspec 'Default' do |ss|
        ss.dependency      'Rhea/Core'
    end

    s.subspec 'Core' do |ss|
        ss.source_files     = 'rhea/**/*.{h,hpp,cpp,c,m}'
    end
end

