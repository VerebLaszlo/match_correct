units = { angle = "degree"; mass = "solar"; };
boundaryFrequency = [40, 500]; samplingFrequency = 10240;

default = {
	binary = {
		mass1 = [ 30.0, 30.0];
		mass2 = [ 30.0, 30.0];
		spin1 = { magnitude = [1.0, 1.0]; inclination = [90.0, 90.0]; azimuth = [0.0, 0.0]; };
		spin2 = { magnitude = [1.0, 1.0]; inclination = [90.0, 90.0]; azimuth = [0.0, 0.0]; };
		inclination = [ 4.0, 4.0]; distance = [ 1.0, 1.0];
	};
	detector = {};
	generation = {
		approximant = "SQT"; phase = 4; spin = "SO"; amplitude = 2;
	};
	name = "wave*2";
};

pairs = (
	(
		{ generation = { approximant = "SQT"; spin = "NONE"; amplitude = 2; phase = 4; } },
		{ generation = { approximant = "ST"; spin = "NONE"; amplitude = 2; phase = 4; } },
		"4no*1"
	),(
		{ generation = { approximant = "SQT"; spin = "SO"; amplitude = 2; phase = 4; } },
		{ generation = { approximant = "ST"; spin = "SO"; amplitude = 2; phase = 4; } },
		"4so*1"
	),(
		{ generation = { approximant = "SQT"; spin = "SS"; amplitude = 2; phase = 4; } },
		{ generation = { approximant = "ST"; spin = "SS"; amplitude = 2; phase = 4; } },
		"4ss*1"
	),(
		{ generation = { approximant = "SQT"; spin = "QM"; amplitude = 2; phase = 4; } },
		{ generation = { approximant = "ST"; spin = "QM"; amplitude = 2; phase = 4; } },
		"4qm*1"
	),(
		{ generation = { approximant = "SQT"; spin = "SELF"; amplitude = 2; phase = 4; } },
		{ generation = { approximant = "ST"; spin = "SELF"; amplitude = 2; phase = 4; } },
		"4se*1"
	),(
		{ generation = { approximant = "SQT"; spin = "SOSS"; amplitude = 2; phase = 4; } },
		{ generation = { approximant = "ST"; spin = "SOSS"; amplitude = 2; phase = 4; } },
		"4soss*1"
	),(
		{ generation = { approximant = "SQT"; spin = "SOQM"; amplitude = 2; phase = 4; } },
		{ generation = { approximant = "ST"; spin = "SOQM"; amplitude = 2; phase = 4; } },
		"4soqm*1"
	),(
		{ generation = { approximant = "SQT"; spin = "SOSELF"; amplitude = 2; phase = 4; } },
		{ generation = { approximant = "ST"; spin = "SOSELF"; amplitude = 2; phase = 4; } },
		"4sose*1"
	),(
		{ generation = { approximant = "SQT"; spin = "SSQM"; amplitude = 2; phase = 4; } },
		{ generation = { approximant = "ST"; spin = "SSQM"; amplitude = 2; phase = 4; } },
		"4ssqm*1"
	),(
		{ generation = { approximant = "SQT"; spin = "SSSELF"; amplitude = 2; phase = 4; } },
		{ generation = { approximant = "ST"; spin = "SSSELF"; amplitude = 2; phase = 4; } },
		"4ssse*1"
	),(
		{ generation = { approximant = "SQT"; spin = "QMSELF"; amplitude = 2; phase = 4; } },
		{ generation = { approximant = "ST"; spin = "QMSELF"; amplitude = 2; phase = 4; } },
		"4qmse*1"
	),(
		{ generation = { approximant = "SQT"; spin = "SOSSQM"; amplitude = 2; phase = 4; } },
		{ generation = { approximant = "ST"; spin = "SOSSQM"; amplitude = 2; phase = 4; } },
		"4sossqm*1"
	),(
		{ generation = { approximant = "SQT"; spin = "SOSSSELF"; amplitude = 2; phase = 4; } },
		{ generation = { approximant = "ST"; spin = "SOSSSELF"; amplitude = 2; phase = 4; } },
		"4sossse*1"
	),(
		{ generation = { approximant = "SQT"; spin = "SSQMSELF"; amplitude = 2; phase = 4; } },
		{ generation = { approximant = "ST"; spin = "SSQMSELF"; amplitude = 2; phase = 4; } },
		"4ssqmse*1"
	),(
		{ generation = { approximant = "SQT"; spin = "SOQMSELF"; amplitude = 2; phase = 4; } },
		{ generation = { approximant = "ST"; spin = "SOQMSELF"; amplitude = 2; phase = 4; } },
		"4soqmse*1"
	),(
		{ generation = { approximant = "SQT"; spin = "SOSSQMSELF"; amplitude = 2; phase = 4; } },
		{ generation = { approximant = "ST"; spin = "SOSSQMSELF"; amplitude = 2; phase = 4; } },
		"4sossqmse*1"
	),(
		{ generation = { approximant = "SQT"; spin = "ALL"; amplitude = 2; phase = 4; } },
		{ generation = { approximant = "ST"; spin = "ALL"; amplitude = 2; phase = 4; } },
		"4all*1"
	)
);
